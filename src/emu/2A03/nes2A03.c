#include "nes2A03.h"
#include <stdlib.h>
#pragma comment(lib, "winmm.lib")

#define INITGUID
#include <initguid.h>

DEFINE_GUID(CLSID_MMDeviceEnumerator,
    0xbcde0395, 0xe52f, 0x467c, 0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e);

DEFINE_GUID(IID_IMMDeviceEnumerator,
    0xa95664d2, 0x9614, 0x4f35, 0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6);

DEFINE_GUID(IID_IAudioClient,
    0x1cb9ad4c, 0xdbfa, 0x4c32, 0xb1, 0x78, 0xc2, 0xf5, 0x3b, 0xd5, 0xc6, 0x86);

DEFINE_GUID(IID_IAudioClient2,
    0x726778CD, 0xF60A, 0x4eda, 0x82, 0xDE,
    0xe4, 0x76, 0x10, 0xcd, 0x78, 0xaa);

DEFINE_GUID(IID_IAudioRenderClient,
    0xf294acfc, 0x3146, 0x4483, 0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2);

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <stdio.h>
#include <platform.h>

#define QUARTZ_AUDIO_MAX_SOUNDS 32
#define QUARTZ_AUDIO_SAMPLE_RATE 48000
#define QUARTZ_AUDIO_NUM_CHANNELS 2

extern u32 quartz_audio_output_sample_rate;

typedef void (*quartz_audio_callback)(f32* output, u32 frame_count);
static quartz_audio_callback external_audio_callback = NULL;

// Allow the NES to register its audio mixing function
void quartz_audio_set_callback(quartz_audio_callback cb) {
    external_audio_callback = cb;
}

u8 quartz_audio_init();
void quartz_audio_shutdown();
void quartz_audio_play_sound(const f32* samples, u32 frame_count, f32 volume);
void quartz_audio_mix(f32* out_buffer, u32 frame_count);

static IAudioClient* audio_client = NULL;
static IAudioRenderClient* render_client = NULL;
static HANDLE audio_event = NULL;
static HANDLE thread_handle = NULL;
static u8 running = false;

#define FRAMES_PER_BUFFER 512

DWORD WINAPI audio_thread_proc(void* param) {
    while (running && platform_should_run()) {
        WaitForSingleObject(audio_event, INFINITE);

        UINT32 padding;
        audio_client->lpVtbl->GetCurrentPadding(audio_client, &padding);

        UINT32 frames_available = FRAMES_PER_BUFFER - padding;
        if (frames_available == 0) { continue; }

        BYTE* data;
        render_client->lpVtbl->GetBuffer(render_client, frames_available, &data);

        // Mix and write to buffer
        quartz_audio_mix((f32*)data, frames_available);

        render_client->lpVtbl->ReleaseBuffer(render_client, frames_available, 0);
    }

    return 0;
}

u8 quartz_audio_init() {
    if (FAILED(CoInitialize(NULL))) { return false; }

    IMMDeviceEnumerator* enumerator = NULL;
    IMMDevice* device = NULL;
    WAVEFORMATEX* format = NULL;

    HRESULT hr = CoCreateInstance(
        &CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        &IID_IMMDeviceEnumerator,
        (void**)&enumerator
    );

    if (FAILED(hr)) { return false; }

    hr = enumerator->lpVtbl->GetDefaultAudioEndpoint(
        enumerator,
        eRender,
        eConsole,
        &device
    );

    if (FAILED(hr)) { return false; }

    hr = device->lpVtbl->Activate(
        device,
        &IID_IAudioClient2,
        CLSCTX_ALL,
        NULL,
        (void**)&audio_client
    );

    if (FAILED(hr)) { return false; }

    hr = audio_client->lpVtbl->GetMixFormat(audio_client, &format);
    if (FAILED(hr)) { return false; }

    quartz_audio_output_sample_rate = format->nSamplesPerSec;

    REFERENCE_TIME buffer_duration = 10000000; // 1 second
    hr = audio_client->lpVtbl->Initialize(
        audio_client,
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        buffer_duration,
        0,
        format,
        NULL
    );

    if (FAILED(hr)) { return false; }

    audio_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    audio_client->lpVtbl->SetEventHandle(audio_client, audio_event);

    hr = audio_client->lpVtbl->GetService(
        audio_client,
        &IID_IAudioRenderClient,
        (void**)&render_client
    );

    if (FAILED(hr)) { return false; }

    audio_client->lpVtbl->Start(audio_client);

    running = true;
    thread_handle = CreateThread(NULL, 0, audio_thread_proc, NULL, 0, NULL);

    CoTaskMemFree(format);
    device->lpVtbl->Release(device);
    enumerator->lpVtbl->Release(enumerator);

    return true;
}

void quartz_audio_shutdown() {
    running = false;
    WaitForSingleObject(thread_handle, INFINITE);
    CloseHandle(thread_handle);
    audio_client->lpVtbl->Stop(audio_client);
    render_client->lpVtbl->Release(render_client);
    audio_client->lpVtbl->Release(audio_client);
    CloseHandle(audio_event);

    CoUninitialize();
}

typedef struct {
    const f32* samples;
    u32 frame_count;
    u32 cursor;
    f32 volume;
    u8 active;
} quartz_audio_sound;

u32 quartz_audio_output_sample_rate = 48000;

static quartz_audio_sound sound_pool[QUARTZ_AUDIO_MAX_SOUNDS] = { 0 };

void quartz_audio_mix(f32* output, u32 frame_count)
{
    memset(output, 0, sizeof(f32) * frame_count * 2);

    if (external_audio_callback) {
        external_audio_callback(output, frame_count);
    }

    for (u32 i = 0; i < QUARTZ_AUDIO_MAX_SOUNDS; ++i) {
        quartz_audio_sound* sound = &sound_pool[i];
        if (!sound->active) continue;

        for (u32 frame = 0; frame < frame_count; ++frame) {
            if (sound->cursor >= sound->frame_count) {
                sound->active = false;
                break;
            }

            f32 l = 0.0f;
            f32 r = 0.0f;
            f32 base_volume = sound->volume;

            l = sound->samples[sound->cursor * 2 + 0] * base_volume;
            r = sound->samples[sound->cursor * 2 + 1] * base_volume;

            output[frame * 2 + 0] += l;
            output[frame * 2 + 1] += r;

            sound->cursor += 1;
        }
    }
}

void quartz_audio_play_sound(const f32* samples, u32 frame_count, f32 volume) {
    for (u32 i = 0; i < QUARTZ_AUDIO_MAX_SOUNDS; ++i) {
        if (!sound_pool[i].active) {
            sound_pool[i] = (quartz_audio_sound){
                .samples = samples,
                .frame_count = frame_count,
                .cursor = 0,
                .volume = volume,
                .active = true,
            };
            break;
        }
    }
}

void nes_audio_mix_callback(f32* output, u32 frame_count);

static const u8 length_table[32] = {
    10, 254, 20,  2, 40,  4, 80,  6,
    160, 8, 60, 10, 14, 12, 26, 14,
    12, 16, 24, 18, 48, 20, 96, 22,
    192, 24, 72, 26, 16, 28, 32, 30
};

nes2A03 nes2A03_init() {
	nes2A03 nes = { 0 };

    if (!quartz_audio_init()) {
        printf("Failed to initialize audio.\n");
        exit(-1);
    }

    quartz_audio_set_callback(nes_audio_mix_callback);

    sequencer_init(&nes.pulse1_seq);
    oscpulse_init(&nes.pulse1_osc);
    envelope_init(&nes.pulse1_env);
    lengthcounter_init(&nes.pulse1_lc);
    sweeper_init(&nes.pulse1_sweep);

    // Initialize pulse2
    sequencer_init(&nes.pulse2_seq);
    oscpulse_init(&nes.pulse2_osc);
    envelope_init(&nes.pulse2_env);
    lengthcounter_init(&nes.pulse2_lc);
    sweeper_init(&nes.pulse2_sweep);

    // Initialize noise
    envelope_init(&nes.noise_env);
    lengthcounter_init(&nes.noise_lc);
    sequencer_init(&nes.noise_seq);
    nes.noise_sample = 0.0;
    nes.noise_output = 0.0;

	nes.noise_seq.sequence = 0xDBDB;

	return nes;
}

void nes2A03_destroy(nes2A03* nes) {
    quartz_audio_shutdown();
}

void nes2A03_cpu_write(nes2A03* nes, uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x4000:
		switch ((data & 0xC0) >> 6)
		{
		case 0x00: nes->pulse1_seq.new_sequence = 0b01000000; nes->pulse1_osc.dutycycle = 0.125; break;
		case 0x01: nes->pulse1_seq.new_sequence = 0b01100000; nes->pulse1_osc.dutycycle = 0.250; break;
		case 0x02: nes->pulse1_seq.new_sequence = 0b01111000; nes->pulse1_osc.dutycycle = 0.500; break;
		case 0x03: nes->pulse1_seq.new_sequence = 0b10011111; nes->pulse1_osc.dutycycle = 0.750; break;
		}
		nes->pulse1_seq.sequence = nes->pulse1_seq.new_sequence;
		nes->pulse1_halt = (data & 0x20);
		nes->pulse1_env.volume = (data & 0x0F);
		nes->pulse1_env.disable = (data & 0x10);
		break;

	case 0x4001:
		nes->pulse1_sweep.enabled = data & 0x80;
		nes->pulse1_sweep.period = (data & 0x70) >> 4;
		nes->pulse1_sweep.down = data & 0x08;
		nes->pulse1_sweep.shift = data & 0x07;
		nes->pulse1_sweep.reload = true;
		break;

	case 0x4002:
		nes->pulse1_seq.reload = (nes->pulse1_seq.reload & 0xFF00) | data;
		break;

	case 0x4003:
		nes->pulse1_seq.reload = (uint16_t)((data & 0x07)) << 8 | (nes->pulse1_seq.reload & 0x00FF);
		nes->pulse1_seq.timer = nes->pulse1_seq.reload;
		nes->pulse1_seq.sequence = nes->pulse1_seq.new_sequence;
		nes->pulse1_lc.counter = length_table[(data & 0xF8) >> 3];
		nes->pulse1_env.start = true;
		break;

	case 0x4004:
		switch ((data & 0xC0) >> 6)
		{
		case 0x00: nes->pulse2_seq.new_sequence = 0b01000000; nes->pulse2_osc.dutycycle = 0.125; break;
		case 0x01: nes->pulse2_seq.new_sequence = 0b01100000; nes->pulse2_osc.dutycycle = 0.250; break;
		case 0x02: nes->pulse2_seq.new_sequence = 0b01111000; nes->pulse2_osc.dutycycle = 0.500; break;
		case 0x03: nes->pulse2_seq.new_sequence = 0b10011111; nes->pulse2_osc.dutycycle = 0.750; break;
		}

		nes->pulse2_seq.sequence = nes->pulse2_seq.new_sequence;
		nes->pulse2_halt = (data & 0x20);
		nes->pulse2_env.volume = (data & 0x0F);
		nes->pulse2_env.disable = (data & 0x10);
		break;

	case 0x4005:
		nes->pulse2_sweep.enabled = data & 0x80;
		nes->pulse2_sweep.period = (data & 0x70) >> 4;
		nes->pulse2_sweep.down = data & 0x08;
		nes->pulse2_sweep.shift = data & 0x07;
		nes->pulse2_sweep.reload = true;
		break;

	case 0x4006:
		nes->pulse2_seq.reload = (nes->pulse2_seq.reload & 0xFF00) | data;
		break;

	case 0x4007:
		nes->pulse2_seq.reload = (uint16_t)((data & 0x07)) << 8 | (nes->pulse2_seq.reload & 0x00FF);
		nes->pulse2_seq.timer = nes->pulse2_seq.reload;
		nes->pulse2_seq.sequence = nes->pulse2_seq.new_sequence;
		nes->pulse2_lc.counter = length_table[(data & 0xF8) >> 3];
		nes->pulse2_env.start = true;

		break;

	case 0x4008:
		break;

	case 0x400C:
		nes->noise_env.volume = (data & 0x0F);
		nes->noise_env.disable = (data & 0x10);
		nes->noise_halt = (data & 0x20);
		break;

	case 0x400E:
		switch (data & 0x0F)
		{
		case 0x00: nes->noise_seq.reload = 0; break;
		case 0x01: nes->noise_seq.reload = 4; break;
		case 0x02: nes->noise_seq.reload = 8; break;
		case 0x03: nes->noise_seq.reload = 16; break;
		case 0x04: nes->noise_seq.reload = 32; break;
		case 0x05: nes->noise_seq.reload = 64; break;
		case 0x06: nes->noise_seq.reload = 96; break;
		case 0x07: nes->noise_seq.reload = 128; break;
		case 0x08: nes->noise_seq.reload = 160; break;
		case 0x09: nes->noise_seq.reload = 202; break;
		case 0x0A: nes->noise_seq.reload = 254; break;
		case 0x0B: nes->noise_seq.reload = 380; break;
		case 0x0C: nes->noise_seq.reload = 508; break;
		case 0x0D: nes->noise_seq.reload = 1016; break;
		case 0x0E: nes->noise_seq.reload = 2034; break;
		case 0x0F: nes->noise_seq.reload = 4068; break;
		}
		break;

	case 0x4015: // APU STATUS
		nes->pulse1_enable = data & 0x01;
		nes->pulse2_enable = data & 0x02;
		nes->noise_enable = data & 0x04;
		break;

	case 0x400F:
		nes->pulse1_env.start = true;
		nes->pulse2_env.start = true;
		nes->noise_env.start = true;
		nes->noise_lc.counter = length_table[(data & 0xF8) >> 3];
		break;
	}
}

uint8_t nes2A03_cpu_read(nes2A03* nes, uint16_t addr)
{
	uint8_t data = 0x00;

	if (addr == 0x4015)
	{
		//	data |= (nes->pulse1_lc.counter > 0) ? 0x01 : 0x00;
		//	data |= (nes->pulse2_lc.counter > 0) ? 0x02 : 0x00;		
		//	data |= (nes->noise_lc.counter > 0) ? 0x04 : 0x00;
	}

	return data;
}

void pulse1_seq_manip_func(uint32_t* sequence) {
	*sequence = ((*sequence & 0x0001) << 7) | ((*sequence & 0x00FE) >> 1);
}

void pulse2_seq_manip_func(uint32_t* sequence) {
	*sequence = ((*sequence & 0x0001) << 7) | ((*sequence & 0x00FE) >> 1);
}

void noise_seq_manip_func(uint32_t* sequence) {
	*sequence = (((*sequence & 0x0001) ^ ((*sequence & 0x0002) >> 1)) << 14) | ((*sequence & 0x7FFF) >> 1);
}

void nes2A03_clock(nes2A03* nes)
{
	bool bQuarterFrameClock = false;
	bool bHalfFrameClock = false;

	nes->dGlobalTime += (0.3333333333 / 1789773);

	if (nes->clock_counter % 6 == 0)
	{
		nes->frame_clock_counter++;

		// 4-Step Sequence Mode
		if (nes->frame_clock_counter == 3729)
		{
			bQuarterFrameClock = true;
		}

		if (nes->frame_clock_counter == 7457)
		{
			bQuarterFrameClock = true;
			bHalfFrameClock = true;
		}

		if (nes->frame_clock_counter == 11186)
		{
			bQuarterFrameClock = true;
		}

		if (nes->frame_clock_counter == 14916)
		{
			bQuarterFrameClock = true;
			bHalfFrameClock = true;
			nes->frame_clock_counter = 0;
		}

		// Update functional units

		// Quater frame "beats" adjust the volume envelope
		if (bQuarterFrameClock)
		{
			envelope_clock(&nes->pulse1_env, nes->pulse1_halt);
			envelope_clock(&nes->pulse2_env, nes->pulse2_halt);
			envelope_clock(&nes->noise_env, nes->noise_halt);
		}


		// Half frame "beats" adjust the note length and
		// frequency sweepers
		if (bHalfFrameClock)
		{
			lengthcounter_clock(&nes->pulse1_lc, nes->pulse1_enable, nes->pulse1_halt);
			lengthcounter_clock(&nes->pulse2_lc, nes->pulse2_enable, nes->pulse2_halt);
			lengthcounter_clock(&nes->noise_lc, nes->noise_enable, nes->noise_halt);
			sweeper_clock(&nes->pulse1_sweep, &nes->pulse1_seq.reload, 0);
			sweeper_clock(&nes->pulse2_sweep, &nes->pulse2_seq.reload, 1);
		}

		//	if (bUseRawMode)
		{
			// Update Pulse1 Channel ================================
			sequencer_clock(&nes->pulse1_seq, nes->pulse1_enable, pulse1_seq_manip_func);

			//	nes->pulse1_sample = (double)nes->pulse1_seq.output;
		}
		//else
		{
			nes->pulse1_osc.frequency = 1789773.0 / (16.0 * (double)(nes->pulse1_seq.reload + 1));
			nes->pulse1_osc.amplitude = (double)(nes->pulse1_env.output - 1) / 16.0;
			nes->pulse1_sample = oscpulse_sample(&nes->pulse1_osc, nes->dGlobalTime);

			if (nes->pulse1_lc.counter > 0 && nes->pulse1_seq.timer >= 8 && !nes->pulse1_sweep.mute && nes->pulse1_env.output > 2)
				nes->pulse1_output += (nes->pulse1_sample - nes->pulse1_output) * 0.5;
			else
				nes->pulse1_output = 0;
		}

		//if (bUseRawMode)
		{
			// Update Pulse1 Channel ================================
			sequencer_clock(&nes->pulse2_seq, nes->pulse2_enable, pulse2_seq_manip_func);

			//	nes->pulse2_sample = (double)nes->pulse2_seq.output;

		}
		//	else
		{
			nes->pulse2_osc.frequency = 1789773.0 / (16.0 * (double)(nes->pulse2_seq.reload + 1));
			nes->pulse2_osc.amplitude = (double)(nes->pulse2_env.output - 1) / 16.0;
			nes->pulse2_sample = oscpulse_sample(&nes->pulse2_osc, nes->dGlobalTime);

			if (nes->pulse2_lc.counter > 0 && nes->pulse2_seq.timer >= 8 && !nes->pulse2_sweep.mute && nes->pulse2_env.output > 2)
				nes->pulse2_output += (nes->pulse2_sample - nes->pulse2_output) * 0.5;
			else
				nes->pulse2_output = 0;
		}

		sequencer_clock(&nes->noise_seq, nes->noise_enable, noise_seq_manip_func);
		
		if (nes->noise_lc.counter > 0 && nes->noise_seq.timer >= 8)
		{
			nes->noise_output = (double)nes->noise_seq.output * ((double)(nes->noise_env.output - 1) / 16.0);
		}

		if (!nes->pulse1_visual) nes->pulse1_output = 0;
		if (!nes->pulse2_enable) nes->pulse2_output = 0;
		if (!nes->noise_enable) nes->noise_output = 0;

	}

	// Frequency sweepers change at high frequency
	sweeper_track(&nes->pulse1_sweep, nes->pulse1_seq.reload);
	sweeper_track(&nes->pulse2_sweep, nes->pulse2_seq.reload);

	nes->pulse1_visual = (nes->pulse1_visual && nes->pulse1_env.output > 1 && !nes->pulse1_sweep.mute) ? nes->pulse1_seq.reload : 2047;
	nes->pulse2_visual = (nes->pulse2_enable && nes->pulse2_env.output > 1 && !nes->pulse2_sweep.mute) ? nes->pulse2_seq.reload : 2047;
	nes->noise_visual = (nes->noise_enable && nes->noise_env.output > 1) ? nes->noise_seq.reload : 2047;

	nes->clock_counter++;
}

double nes2A03_get_output_sample(nes2A03* nes)
{
	if (nes->bUseRawMode)
	{
		return (nes->pulse1_sample - 0.5) * 0.5
			+ (nes->pulse2_sample - 0.5) * 0.5;
	}
	else
	{
		return ((1.0 * nes->pulse1_output) - 0.8) * 0.1 +
			((1.0 * nes->pulse2_output) - 0.8) * 0.1 +
			((2.0 * (nes->noise_output - 0.5))) * 0.1;
	}
}
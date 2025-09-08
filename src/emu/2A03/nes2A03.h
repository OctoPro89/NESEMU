#pragma once
#include <emu/common.h>

#include <stdint.h>
#include <stdbool.h>

// --- sequencer ---

typedef struct {
    uint32_t sequence;
    uint32_t new_sequence;
    uint16_t timer;
    uint16_t reload;
    uint8_t output;
} sequencer;

typedef void (*manip_func)(uint32_t* sequence);

inline void sequencer_init(sequencer* seq) {
    seq->sequence = 0x00000000;
    seq->new_sequence = 0x00000000;
    seq->timer = 0x0000;
    seq->reload = 0x0000;
    seq->output = 0x00;
}

inline uint8_t sequencer_clock(sequencer* seq, bool bEnable, manip_func funcManip) {
    if (bEnable) {
        seq->timer--;

        if (seq->timer == 0xFFFF) {
            seq->timer = seq->reload;
            funcManip(&seq->sequence);
            seq->output = seq->sequence & 0x00000001;
        }
    }
    return seq->output;
}

// Example manipulation function: rotate left by 1 bit
inline void manip_rotate_left(uint32_t* sequence) {
    *sequence = (*sequence << 1) | ((*sequence >> 31) & 1);
}

// --- lengthcounter ---

typedef struct {
    uint8_t counter;
} lengthcounter;

inline void lengthcounter_init(lengthcounter* lc) {
    lc->counter = 0x00;
}

inline uint8_t lengthcounter_clock(lengthcounter* lc, bool bEnable, bool bHalt) {
    if (!bEnable)
        lc->counter = 0;
    else
        if (lc->counter > 0 && !bHalt)
            lc->counter--;
    return lc->counter;
}

// --- envelope ---

typedef struct {
    bool start;
    bool disable;
    uint16_t divider_count;
    uint16_t volume;
    uint16_t output;
    uint16_t decay_count;
} envelope;

inline void envelope_init(envelope* env) {
    env->start = false;
    env->disable = false;
    env->divider_count = 0;
    env->volume = 0;
    env->output = 0;
    env->decay_count = 0;
}

inline void envelope_clock(envelope* env, bool bLoop) {
    if (!env->start)
    {
        if (env->divider_count == 0)
        {
            env->divider_count = env->volume;

            if (env->decay_count == 0)
            {
                if (bLoop)
                {
                    env->decay_count = 15;
                }

            }
            else
                env->decay_count--;
        }
        else
            env->divider_count--;
    }
    else
    {
        env->start = false;
        env->decay_count = 15;
        env->divider_count = env->volume;
    }

    if (env->disable)
    {
        env->output = env->volume;
    }
    else
    {
        env->output = env->decay_count;
    }
}

// --- oscpulse ---

typedef struct {
    double frequency;
    double dutycycle;
    double amplitude;
    double pi;
    double harmonics;
} oscpulse;

inline void oscpulse_init(oscpulse* osc) {
    osc->frequency = 0.0;
    osc->dutycycle = 0.0;
    osc->amplitude = 1.0;
    osc->pi = 3.14159;
    osc->harmonics = 20.0;
}

inline static double approxsin(double t) {
    double j = t * 0.15915;
    j = j - (int)j;
    return 20.785 * j * (j - 0.5) * (j - 1.0);
}

inline double oscpulse_sample(oscpulse* osc, double t) {
    double a = 0.0;
    double b = 0.0;
    double p = osc->dutycycle * 2.0 * osc->pi;
    for (int n = 1; n < (int)osc->harmonics; n++) {
        double c = n * osc->frequency * 2.0 * osc->pi * t;
        a += -approxsin(c) / n;
        b += -approxsin(c - p * n) / n;
    }
    return (2.0 * osc->amplitude / osc->pi) * (a - b);
}

// --- sweeper ---

typedef struct {
    bool enabled;
    bool down;
    bool reload;
    uint8_t shift;
    uint8_t timer;
    uint8_t period;
    uint16_t change;
    bool mute;
} sweeper;

inline void sweeper_init(sweeper* sw) {
    sw->enabled = false;
    sw->down = false;
    sw->reload = false;
    sw->shift = 0x00;
    sw->timer = 0x00;
    sw->period = 0x00;
    sw->change = 0;
    sw->mute = false;
}

inline void sweeper_track(sweeper* sw, uint16_t target) {
    if (sw->enabled) {
        sw->change = target >> sw->shift;
        sw->mute = (target < 8) || (target > 0x7FF);
    }
}

inline bool sweeper_clock(sweeper* sw, uint16_t* target, bool channel) {
    bool changed = false;

    if (sw->timer == 0 && sw->enabled && sw->shift > 0 && !sw->mute) {
        if (*target >= 8 && sw->change < 0x07FF) {
            if (sw->down) {
                *target -= sw->change - channel;
            }
            else {
                *target += sw->change;
            }
            changed = true;
        }
    }

    if (sw->timer == 0 || sw->reload) {
        sw->timer = sw->period;
        sw->reload = false;
    }
    else {
        sw->timer--;
    }

    sw->mute = (*target < 8) || (*target > 0x7FF);

    return changed;
}

typedef struct {
    double dGlobalTime;

    // Pulse 1
    bool pulse1_enable;
    bool pulse1_halt;
    double pulse1_sample;
    double pulse1_output;
    sequencer pulse1_seq;
    oscpulse pulse1_osc;
    envelope pulse1_env;
    lengthcounter pulse1_lc;
    sweeper pulse1_sweep;

    // Pulse 2
    bool pulse2_enable;
    bool pulse2_halt;
    double pulse2_sample;
    double pulse2_output;
    sequencer pulse2_seq;
    oscpulse pulse2_osc;
    envelope pulse2_env;
    lengthcounter pulse2_lc;
    sweeper pulse2_sweep;

    // Noise
    bool noise_enable;
    bool noise_halt;
    envelope noise_env;
    lengthcounter noise_lc;
    sequencer noise_seq;
    double noise_sample;
    double noise_output;

    uint16_t pulse1_visual;
    uint16_t pulse2_visual;
    uint16_t noise_visual;
    uint16_t triangle_visual;
    uint32_t frame_clock_counter;
    uint32_t clock_counter;
    bool bUseRawMode;

    double pulse1_filtered;
    double pulse2_filtered;
    double noise_filtered;
} nes2A03;

nes2A03 nes2A03_init();
void nes2A03_destroy(nes2A03* nes);

u8 nes2A03_cpu_read(nes2A03* nes, u16 addr);
void nes2A03_cpu_write(nes2A03* nes, u16 addr, u8 data);

void nes2A03_clock(nes2A03* nes);

double nes2A03_get_output_sample(nes2A03* nes);
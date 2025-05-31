#include "emu.h"
#include "bus.h"

#include <stdio.h>

int running = 1;

// --- INPUT ---

typedef struct {
    u8 up, down, left, right;
    u8 space, shift, enter;
    u8 x, z;
} keys;

keys keyboard;

// --- INPUT ---

#ifdef _POSIX_C_SOURCE

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>  // nanosleep
#else
#include <unistd.h>  // usleep
#endif

void platform_sleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, 0);
#else
    if (ms >= 1000) {
        sleep(ms / 1000);
    }
    usleep((ms % 1000) * 1000);
#endif
}

f64 get_absolute_time() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}

// --- GRAPHICS ---

#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glad/glad.h>
#include <GL/glx.h>

#define WIDTH 256
#define HEIGHT 240

Display *display;
Window window;
GLXContext glContext;
Atom WM_DELETE_WINDOW;
XEvent event;
GLuint program;
GLuint vao, vbo;
GLuint texture;

uint8_t framebuffer[WIDTH * HEIGHT * 3]; // RGB software framebuffer

// Vertex + texture coordinate shader
const char *vertexShaderSource =
    "#version 130\n"
    "in vec2 position;\n"
    "in vec2 texCoord;\n"
    "out vec2 TexCoord;\n"
    "void main() {\n"
    "    TexCoord = texCoord;\n"
    "    gl_Position = vec4(position, 0.0, 1.0);\n"
    "}\n";

const char *fragmentShaderSource =
    "#version 130\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D screenTexture;\n"
    "void main() {\n"
    "    FragColor = texture(screenTexture, TexCoord);\n"
    "}\n";

void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    int index = (y * WIDTH + x) * 3;
    framebuffer[index + 0] = r;
    framebuffer[index + 1] = g;
    framebuffer[index + 2] = b;
}

void createWindow(int width, int height) {
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X display\n");
        exit(1);
    }

    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    XVisualInfo vi;
    XMatchVisualInfo(display, screen, 24, TrueColor, &vi);

    XSetWindowAttributes swa;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;

    window = XCreateWindow(display, root, 0, 0, width, height, 0,
                           vi.depth, InputOutput, vi.visual, CWEventMask, &swa);
    XMapWindow(display, window);
    XFlush(display);

    glContext = glXCreateContext(display, &vi, NULL, GL_TRUE);
    glXMakeCurrent(display, window, glContext);

    WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);

    if (!gladLoadGLLoader((GLADloadproc)glXGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        exit(1);
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

GLuint compileShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        fprintf(stderr, "Shader error: %s\n", log);
        exit(1);
    }
    return shader;
}

GLuint createShaderProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void setupQuad() {
    float quadVertices[] = {
        // positions   // tex coords
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    GLuint vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void setupTexture() {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // NES-style
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void updateTexture() {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindVertexArray(vao);
    updateTexture();
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glXSwapBuffers(display, window);
}

void cleanup() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texture);
    glDeleteProgram(program);
    glXMakeCurrent(display, None, NULL);
    glXDestroyContext(display, glContext);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

// --- GRAPHICS ---

#endif // _POSIX_C_SOURCE

#ifdef _WIN64

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glad/glad.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

f64 get_absolute_time() {
    return (f64)GetTickCount64();
}

// --- GRAPHICS ---

#define WIDTH 256
#define HEIGHT 240

HDC hDC;
HGLRC hRC;
HWND hWnd;
HINSTANCE hInstance;
GLuint program;
GLuint vao, vbo;
GLuint texture;

LPCWSTR class_name = L"NESEMUWINDOWCLASS";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

uint8_t framebuffer[WIDTH * HEIGHT * 3]; // RGB software framebuffer

// Vertex + texture coordinate shader
const char* vertexShaderSource =
"#version 130\n"
"in vec2 position;\n"
"in vec2 texCoord;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"    TexCoord = texCoord;\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"}\n";

const char* fragmentShaderSource =
"#version 130\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D screenTexture;\n"
"void main() {\n"
"    FragColor = texture(screenTexture, TexCoord);\n"
"}\n";

u8 SetupPixelFormat(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or color index.
        32,                   // Color depth
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Depth buffer
        8,                    // Stencil buffer
        0,                    // Auxiliary buffer
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0) return false;

    return SetPixelFormat(hdc, pixelFormat, &pfd);
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
        case WM_DESTROY:
            running = false;
            return 0;
        case WM_KEYDOWN:
            switch (wParam) {
            case VK_SHIFT:
                keyboard.shift = true;
                break;
            case VK_RETURN:
                keyboard.enter = true;
                break;
            case VK_UP:
                keyboard.up = true;
                break;
            case VK_DOWN:
                keyboard.down = true;
                break;
            case VK_LEFT:
                keyboard.left = true;
                break;
            case VK_RIGHT:
                keyboard.right = true;
                break;
            case 'z':
                keyboard.z = true;
                break;
            case 'x':
                keyboard.x = true;
                break;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    int index = (y * WIDTH + x) * 3;
    framebuffer[index + 0] = r;
    framebuffer[index + 1] = g;
    framebuffer[index + 2] = b;
}

void createWindow(int width, int height) {
    hInstance = GetModuleHandleA(NULL);

    WNDCLASS wc = { 0 };
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"RegisterClass failed!", L"Error", MB_OK);
        exit(-1);
    }

    hWnd = CreateWindowEx(
        0, class_name, L"NESEMU",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        MessageBox(NULL, L"CreateWindowEx failed!", L"Error", MB_OK);
        exit(-1);
    }

    hDC = GetDC(hWnd);

    if (!SetupPixelFormat(hDC)) {
        MessageBox(NULL, L"Failed to set pixel format.", L"Error", MB_OK);
        exit(-1);
    }

    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    // Load GLAD
    if (!gladLoadGL()) {
        MessageBox(NULL, L"Failed to initialize GLAD", L"Error", MB_OK);
        exit(-1);
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        fprintf(stderr, "Shader error: %s\n", log);
        exit(1);
    }
    return shader;
}

GLuint createShaderProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void setupQuad() {
    float quadVertices[] = {
        // positions   // tex coords
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    GLuint vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void setupTexture() {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // NES-style
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void updateTexture() {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindVertexArray(vao);
    updateTexture();
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    SwapBuffers(hDC);
}

void cleanup() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texture);
    glDeleteProgram(program);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
    DestroyWindow(hWnd);
}

// --- GRAPHICS ---

#endif // _WIN64

u8 emu_run() {
    const char* cartridge_fp = "balloon_fight.nes";

    createWindow(WIDTH * 4, HEIGHT * 4); // Upscale window

    program = createShaderProgram();

    setupQuad();
    setupTexture();

    bus nes = bus_init();

    cartridge cart = cartridge_init(cartridge_fp);

    if (!cartridge_image_valid(&cart)) {
        printf("Invalid cartridge image! (Failed to load cartridge)\n");
        return false;
    }

    printf("Loaded cart: %s\n", cartridge_fp);

    bus_insert_cartridge(&nes, &cart);

    bus_reset(&nes);
    bus_set_sample_frequency(&nes, 44100);

    f32 now_time = 0.0f;
    f32 residual_time = 0.0f;
    f32 elapsed_time = 0.0f;

    while (running) {
#ifdef _POSIX_C_SOURCE

        while (XPending(display)) {
            XNextEvent(display, &event);
            
            switch (event.type) {
                case KeyPress: {
                    // Filter out auto-repeat key presses
                    if (XEventsQueued(display, QueuedAfterReading)) {
                        XEvent next_event;
                        XPeekEvent(display, &next_event);

                        if (next_event.type == KeyRelease &&
                            next_event.xkey.time == event.xkey.time &&
                            next_event.xkey.keycode == event.xkey.keycode) {
                            // This is an auto-repeat, skip it
                            break;
                        }
                    }

                    KeySym k = XLookupKeysym(&event.xkey, 0);

                    switch (k) {
                        case XK_space: {
                            keyboard.space = true;
                            break;
                        }
                        case XK_Up: {
                            keyboard.up = true;
                            break;
                        }
                        case XK_Down: {
                            keyboard.down = true;
                            break;
                        }
                        case XK_Left: {
                            keyboard.left = true;
                            break;
                        }
                        case XK_Right: {
                            keyboard.right = true;
                            break;
                        }
                        case XK_x: {
                            keyboard.x = true;
                            break;
                        }
                        case XK_z: {
                            keyboard.z = true;
                            break;
                        }
                        case XK_Return: {
                            keyboard.enter = true;
                            break;
                        }
                        case XK_Shift_R: {
                            keyboard.shift = true;
                            break;
                        }
                    }

                    break;
                }

                case KeyRelease: {
                    KeySym k = XLookupKeysym(&event.xkey, 0);

                    switch (k) {
                        case XK_space: {
                            keyboard.space = false;
                            break;
                        }
                        case XK_Up: {
                            keyboard.up = false;
                            break;
                        }
                        case XK_Down: {
                            keyboard.down = false;
                            break;
                        }
                        case XK_Left: {
                            keyboard.left = false;
                            break;
                        }
                        case XK_Right: {
                            keyboard.right = false;
                            break;
                        }
                        case XK_x: {
                            keyboard.x = false;
                            break;
                        }
                        case XK_z: {
                            keyboard.z = false;
                            break;
                        }
                        case XK_Return: {
                            keyboard.enter = false;
                            break;
                        }
                        case XK_Shift_R: {
                            keyboard.shift = false;
                            break;
                        }
                    }
                }

                break;

                case ClientMessage: {
                    if ((Atom)event.xclient.data.l[0] == WM_DELETE_WINDOW) {
                        running = 0;
                    }

                    break;
                }

                break;
            }
        }

#endif // _POSIX_C_SOURCE

#ifdef _WIN64
    
        MSG msg = { 0 };
        while (GetMessage(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

#endif // _WIN64
        nes.controller[0] =
            (keyboard.z << 0) |
            (keyboard.x << 1) |
            (keyboard.down << 2) |
            (keyboard.up << 3) |
            (keyboard.enter << 4) |
            (keyboard.shift << 5) |
            (keyboard.left << 6) |
            (keyboard.right << 7);

        now_time = (f32)get_absolute_time();
        if (residual_time > 0.0f) {
            residual_time -= elapsed_time;
        }
        else {
            residual_time += (1.0f / 60.0f) - elapsed_time;
            do {
                // nes6502_disassemble(&nes.cpu, nes.cpu.pc, nes.cpu.pc + 0x0002);
                bus_clock(&nes); 
            } while (!nes.ppu.frame_complete);
            nes.ppu.frame_complete = false;

            elapsed_time = (f32)get_absolute_time();
        }

        // Draw rendered output
        for (u32 j = 0; j < HEIGHT; ++j) {
            for (u32 i = 0; i < WIDTH; ++i) {
                u32 index = j * WIDTH + i;
                pixel* p = &nes.ppu.screen[index];

                // Flip the Y-coordinate
                u32 flipped_j = HEIGHT - 1 - j;

                set_pixel(i, flipped_j, p->r, p->g, p->b);
            }
        }

        render();
    }

    cleanup();

    bus_destroy(&nes);

    return true;
}
//
//  obj_c_platform_interop.m
//  NESEMU
//
//  Created by Ansley Scoglietti on 6/2/25.
//

// shutup apple i'll play my nes games if I want
#define GLES_SILENCE_DEPRECATION

#import "obj_c_platform_interop.h"
#import "obj_c_interop.h"

#include <platform.h>

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import <OpenGLES/ES3/gl.h>
#import <QuartzCore/QuartzCore.h>
#import <mach/mach_time.h>

keys touch_input;

static PlatformViewController *pvc_inst = nil;;

#define NES_WIDTH 256
#define NES_HEIGHT 240

u8 framebuffer[NES_WIDTH * NES_HEIGHT * 3];

@implementation PlatformViewController {
    EAGLContext* context;
    GLuint shader_program, vao, vbo, texture;
    UIView *dpad;
    UIButton *buttonA;
    UIButton *buttonB;
    UIButton *start;
    UIButton *select;
}

u8 ios_init(void) {
    return true;
}

void ios_render(void) {
    
}

void ios_sleep_ms(u32 ms) {
    usleep((useconds_t)(ms * 1000));
}

f64 ios_get_elapsed_ms(void) {
    static mach_timebase_info_data_t timebase;
    static u64 start;
    
    if (start == 0) {
        mach_timebase_info(&timebase);
        start = mach_absolute_time();
    }
    
    u64 now = mach_absolute_time();
    u64 elapsed = now - start;
    f64 ms = (f64)(elapsed * timebase.numer) / (timebase.denom * 1e6);
    return ms;
}

void ios_put_pixel(i32 x, i32 y, u8 r, u8 g, u8 b) {
    if (x < 0 || x >= NES_WIDTH || y < 0 || y >= NES_HEIGHT) return;
    i32 index = (y * NES_WIDTH + x) * 3;
    framebuffer[index] = r;
    framebuffer[index + 1] = g;
    framebuffer[index + 2] = b;
}

keys ios_get_touch_input(void) {
    return touch_input;
}

const char* vertex_shader_source =
"#version 300 es\n"
"layout(location = 0) in vec2 position;\n"
"layout(location = 1) in vec2 texCoord;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"   TexCoord = texCoord;\n"
"   gl_Position = vec4(position, 0.0, 1.0);\n"
"}";

const char* fragment_shader_source =
"#version 300 es\n"
"precision mediump float;\n"
"in vec2 TexCoord;"
"out vec4 FragColor;\n"
"uniform sampler2D screenTexture;\n"
"void main() {\n"
"   FragColor = texture(screenTexture, TexCoord);\n"
"}";

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)loadView {
    // GL Context
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    GLKView* view = [[GLKView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
    view.contentScaleFactor = [UIScreen mainScreen].scale;
    view.context = context;
    view.enableSetNeedsDisplay = YES;
    view.delegate = self;
    self.preferredFramesPerSecond = 60;
    [EAGLContext setCurrentContext:context];
    self.view = view;
    
    [self setup_shaders];
    [self setup_quad];
    [self setup_texture];
    
    [self add_touch_controls];
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    view.userInteractionEnabled = YES;
    self.view.multipleTouchEnabled = YES;
    
    pvc_inst = self;
}

- (void)add_touch_controls {
    // Common dimensions
    CGFloat button_size = 57.5f;
    CGFloat margin = 20.0f;
    CGFloat offsetY = 60.0f;
    CGFloat screenW = self.view.bounds.size.width;
    CGFloat screenH = self.view.bounds.size.height;

    // === D-PAD ===
    dpad = [[UIView alloc] initWithFrame:CGRectMake(margin, screenH - 3 * button_size - margin - offsetY, button_size * 3, button_size * 3)];
    dpad.backgroundColor = [UIColor clearColor];
    
    // Directions
    [self add_directional_button:@"↑" dx:1 dy:0 tag:100 to:dpad];
    [self add_directional_button:@"↓" dx:1 dy:2 tag:101 to:dpad];
    [self add_directional_button:@"←" dx:0 dy:1 tag:102 to:dpad];
    [self add_directional_button:@"→" dx:2 dy:1 tag:103 to:dpad];
    
    [self.view addSubview:dpad];

    // === A / B Buttons ===
    CGFloat rightStartX = screenW - margin - button_size * 2.2;
    CGFloat bottomY = screenH - button_size * 2 - margin;

    buttonA = [self create_button_with_title:@"A" tag:200];
    buttonA.frame = CGRectMake(rightStartX + button_size, bottomY - offsetY, button_size, button_size);
    buttonA.layer.cornerRadius = button_size / 2.0f;
    buttonA.backgroundColor = [UIColor redColor];
    
    buttonB = [self create_button_with_title:@"B" tag:201];
    buttonB.frame = CGRectMake(rightStartX - (button_size / 2), bottomY - offsetY, button_size, button_size);
    buttonB.layer.cornerRadius = button_size / 2.0f;
    buttonB.backgroundColor = [UIColor redColor];

    [self.view addSubview:buttonA];
    [self.view addSubview:buttonB];

    // === Start / Select ===
    start = [self create_button_with_title:@"Start" tag:202];
    start.frame = CGRectMake(screenW/2 + 20, screenH - button_size - margin, button_size, button_size * 0.8);

    select = [self create_button_with_title:@"Select" tag:203];
    select.frame = CGRectMake(screenW/2 - button_size - 20, screenH - button_size - margin, button_size, button_size * 0.8);

    [self.view addSubview:start];
    [self.view addSubview:select];
}

- (UIButton *)create_button_with_title:(NSString *)title tag:(NSInteger)tag {
    UIButton *btn = [UIButton buttonWithType:UIButtonTypeSystem];
    [btn setTitle:title forState:UIControlStateNormal];
    [btn setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    btn.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent:0.5];
    btn.layer.cornerRadius = 15;
    btn.tag = tag;

    [btn addTarget:self action:@selector(button_down:) forControlEvents:UIControlEventTouchDown];
    [btn addTarget:self action:@selector(button_up:) forControlEvents:UIControlEventTouchUpInside | UIControlEventTouchUpOutside];

    return btn;
}

- (void)add_directional_button:(NSString *)title dx:(NSInteger)dx dy:(NSInteger)dy tag:(NSInteger)tag to:(UIView *)parent {
    CGFloat size = 57.5f;
    UIButton *btn = [self create_button_with_title:title tag:tag];
    btn.frame = CGRectMake(dx * size, dy * size, size, size);
    btn.layer.cornerRadius = size / 2.0f;
    [parent addSubview:btn];
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    
    CGFloat screenW = self.view.bounds.size.width;
    CGFloat screenH = self.view.bounds.size.height;
    
    BOOL is_landscape = screenW > screenH;

    CGFloat button_size = is_landscape ? 62.0f : 57.5f;
    CGFloat margin = 20.0;

    // === D-PAD Layout ===
    dpad.frame = CGRectMake(margin, screenH - 3 * button_size - margin - 60.0f, button_size * 3, button_size * 3);

    // Move directional buttons inside dpad
    for (UIButton *btn in dpad.subviews) {
        switch (btn.tag) {
            case 100: btn.frame = CGRectMake(button_size, 0 * button_size, button_size, button_size); break;   // ↑
            case 101: btn.frame = CGRectMake(button_size, 2 * button_size, button_size, button_size); break;   // ↓
            case 102: btn.frame = CGRectMake(0 * button_size, button_size, button_size, button_size); break;   // ←
            case 103: btn.frame = CGRectMake(2 * button_size, button_size, button_size, button_size); break;   // →
        }
    }

    // === A / B Buttons ===
    CGFloat rightStartX = screenW - margin - button_size * 2.2;
    CGFloat bottomY = screenH - button_size * 2 - margin;

    buttonA.frame = CGRectMake(rightStartX + button_size - (is_landscape ? 10.0f : 0.0f), bottomY - 60.0f, button_size, button_size);
    buttonB.frame = CGRectMake(rightStartX - (button_size / 2) - (is_landscape ? 10.0f : 0.0f), bottomY - 60.0f, button_size, button_size);

    // === Start / Select ===
    start.frame = CGRectMake(screenW/2 + (is_landscape ? 180 : 20), screenH - button_size - margin, button_size, button_size * 0.8);
    select.frame = CGRectMake(screenW/2 - button_size - (is_landscape ? 180 : 20), screenH - button_size - margin, button_size, button_size * 0.8);
}


- (void)button_down:(UIButton *)sender {
    switch (sender.tag) {
        case 100: {
            touch_input.up = true;
            break;
        }
        case 101: {
            touch_input.down = true;
            break;
        }
        case 102: {
            touch_input.left = true;
            break;
        }
        case 103: {
            touch_input.right = true;
            break;
        }
        case 200: {
            touch_input.x = true;
            break;
        }
        case 201: {
            touch_input.z = true;
            break;
        }
        case 202: {
            touch_input.enter = true;
            break;
        }
        case 203: {
            touch_input.shift = true;
            break;
        }
    }
}

- (void)button_up:(UIButton *)sender {
    switch (sender.tag) {
        case 100: {
            touch_input.up = false;
            break;
        }
        case 101: {
            touch_input.down = false;
            break;
        }
        case 102: {
            touch_input.left = false;
            break;
        }
        case 103: {
            touch_input.right = false;
            break;
        }
        case 200: {
            touch_input.x = false;
            break;
        }
        case 201: {
            touch_input.z = false;
            break;
        }
        case 202: {
            touch_input.enter = false;
            break;
        }
        case 203: {
            touch_input.shift = false;
            break;
        }
    }
}

- (void)setup_shaders {
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

- (void)setup_quad {
    f32 vertices[] = {
        -1.0f,  1.0f,    0.0f, 1.0f,
        -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
        
        -1.0f,  1.0f,    0.0f, 1.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
         1.0f,  1.0f,    1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

- (void)setup_texture {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, NES_WIDTH, NES_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // NES look
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    i32 scale = (i32)MIN(view.drawableWidth / NES_WIDTH, view.drawableHeight / NES_HEIGHT);
    i32 render_width = NES_WIDTH * scale;
    i32 render_height = NES_HEIGHT * scale;
    i32 offset_x = (i32)(view.drawableWidth - render_width) / 2;
    i32 offset_y = (i32)(view.drawableHeight - render_height) / 2;
    
    glViewport(offset_x, offset_y, render_width, render_height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    
    // Update texture with emulator framebuffer
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, NES_WIDTH, NES_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

- (void)dealloc {
    if (shader_program) {
        glDeleteProgram(shader_program);
        shader_program = 0;
    }
    
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    
    if (texture) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
    
    GLKView* v = (GLKView*)self.view;
    if ([EAGLContext currentContext] == v.context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    v.context = nil;
}

@end

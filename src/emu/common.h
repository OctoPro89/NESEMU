#pragma once
#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define f32 float
#define f64 double

#define true 1
#define false 0

typedef struct {
    u8 r, g, b;
} pixel;

typedef struct {
    u8 z, x, enter, shift, up, down, left, right, space, f, c, u;
} keys;

#define NES_WIDTH 256
#define NES_HEIGHT 240

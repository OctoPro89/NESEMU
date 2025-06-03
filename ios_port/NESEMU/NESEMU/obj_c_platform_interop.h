//
//  obj_c_platform_interop.h
//  NESEMU
//
//  Created by Ansley Scoglietti on 6/2/25.
//

#ifndef obj_c_platform_interop_h
#define obj_c_platform_interop_h

#include <emu/common.h>

extern u8 ios_init(void);
extern void ios_render(void);
extern void ios_sleep_ms(u32 ms);
extern f64 ios_get_elapsed_ms(void);
extern void ios_put_pixel(i32 x, i32 y, u8 r, u8 g, u8 b);
extern keys ios_get_touch_input(void);

#endif /* obj_c_platform_interop_h */

//
//  obj_c_interop.h
//  NESEMU
//
//  Created by Ansley Scoglietti on 6/2/25.
//

#ifndef obj_c_interop_h
#define obj_c_interop_h

#import <GLKit/GLKit.h>

@interface PlatformViewController : GLKViewController
    void setup_shaders(void);
    void setup_quad(void);
    void setup_texture(void);
@end

#endif /* obj_c_interop_h */

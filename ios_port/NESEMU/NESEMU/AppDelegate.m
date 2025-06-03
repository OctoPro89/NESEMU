//
//  AppDelegate.m
//  NESEMU
//
//  Created by Ansley Scoglietti on 6/2/25.
//

#import "AppDelegate.h"
#import "obj_c_interop.h"
#include <emu/emu.h>

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)copyInitialFilesToDocumentsIfNeeded {
    NSArray *testFiles = @[@"test.txt"];
    NSFileManager *fm = [NSFileManager defaultManager];
    
    NSURL *docsURL = [[fm URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] firstObject];
    
    for (NSString *file in testFiles) {
        NSString *name = [file stringByDeletingPathExtension];
        NSString *ext = [file pathExtension];
        NSURL *bundleURL = [[NSBundle mainBundle] URLForResource:name withExtension:ext];
        NSURL *destURL = [docsURL URLByAppendingPathComponent:file];
        
        if (![fm fileExistsAtPath:destURL.path]) {
            NSError *error;
            [fm copyItemAtURL:bundleURL toURL:destURL error:&error];
            if (error) {
                NSLog(@"Error copying %@: %@", file, error.localizedDescription);
            } else {
                NSLog(@"Copied %@ to Documents", file);
            }
        }
    }
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // [self copyInitialFilesToDocumentsIfNeeded];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSArray<NSString *> *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documents_directory = paths.firstObject;
        NSString *rom_fp = [documents_directory stringByAppendingPathComponent:@"rom.nes"];
        
        const char* cart_fp = [rom_fp UTF8String];
        
        if (!emu_run(cart_fp)) {
            printf("Failed to start emulator!\n");
            exit(1);
        }
    });
    
    return YES;
}


#pragma mark - UISceneSession lifecycle


- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options {
    // Called when a new scene session is being created.
    // Use this method to select a configuration to create the new scene with.
    return [[UISceneConfiguration alloc] initWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}


- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {
    // Called when the user discards a scene session.
    // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
    // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
}


@end

#import "AppDelegate.h"
#import "ViewController.h"

ViewController *getViewController(){
    return (ViewController *)[[NSApplication sharedApplication] mainWindow].contentViewController;
}

@interface AppDelegate ()

@end

@implementation AppDelegate
- (IBAction)encryptMenuItem:(id)sender {
    [getViewController() splitFile:self];
}

- (IBAction)decryptMenuItem:(id)sender {
    [getViewController() joinFile:self];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

@end

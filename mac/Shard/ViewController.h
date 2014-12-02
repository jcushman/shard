#import <Cocoa/Cocoa.h>
#include "shardshare.h"

@interface ViewController : NSViewController

@property (weak) IBOutlet NSSlider *shardCountSlider;
@property (weak) IBOutlet NSSlider *thresholdSlider;
@property (weak) IBOutlet NSTextField *shardCountLabel;
@property (weak) IBOutlet NSTextField *thresholdLabel;
- (IBAction)joinFile:(id)sender;
- (IBAction)splitFile:(id)sender;

@end


/*
 TODO:
 - handle requests to encrypt packages, which are really directories
    - suggest zipping?
    - OR allow encrypting of directory by zipping?
 - handle too few shards selected in join
 - handle potential overwrite of existing files
 - progress bar/cancel button
 */

#import "ViewController.h"
#include "shardshare.h"

void
showAlert(NSString *message, NSString *text){
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = message;
    alert.informativeText = text;
    [alert runModal];
}

@implementation ViewController


- (IBAction)splitFile:(id)sender {
    NSOpenPanel *dialog = [NSOpenPanel openPanel];
    dialog.allowsMultipleSelection = false;
    dialog.canChooseDirectories = false;
    dialog.prompt = @"Encrypt File";
    dialog.title = @"Encrypt File";
    dialog.message = @"Choose the file to encrypt:";
    if([dialog runModal] == NSFileHandlingPanelOKButton){
        int resultCode = do_gfsplit((int)_shardCountSlider.integerValue, (int)_thresholdSlider.integerValue, dialog.URL.path.UTF8String, dialog.URL.path.UTF8String);
        // TODO: handle failed error code
        // showAlert(@"Done!", [NSString stringWithFormat:@"%i encrypted shards created.", (int)_shardCountSlider.integerValue]);
        // show new files in Finder
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:[NSArray arrayWithObjects:[dialog.URL URLByAppendingPathExtension:@"1.shard"], nil]];
    }
}

- (IBAction)joinFile:(id)sender {
    NSOpenPanel *dialog = [NSOpenPanel openPanel];
    dialog.allowsMultipleSelection = true;
    dialog.canChooseDirectories = false;
    dialog.prompt = @"Decrypt Shards";
    dialog.title = @"Decrypt Shards";
    dialog.message = @"Choose the shards to decrypt (use Command or Shift key to select multiple):";
    if([dialog runModal] == NSFileHandlingPanelOKButton){
        const char **paths, **p;
        p = paths = malloc(sizeof(*paths) * dialog.URLs.count);
        for(NSURL *url in dialog.URLs){
            *p++ = strdup(url.path.UTF8String);
        }
        struct shardshare_headers *headers = get_headers(paths[0]);
        // TODO: (1) check if headers were read successfully; (2) check if enough files were provided
        
        NSString *resultPath = [[[dialog.URLs objectAtIndex:0] URLByDeletingLastPathComponent] URLByAppendingPathComponent:@(headers->original_file_name)].path;
        // TODO: check for overwrite of existing file
        
        int resultCode = do_gfcombine(resultPath.UTF8String, paths, dialog.URLs.count);
        // TODO: check result code
        
        p = paths;
        for (int i=0; i<dialog.URLs.count; i++)
            free(*p++);
        free(paths);
        free_headers(headers);
        
        // showAlert(@"Done!", @"");
        // show new file in Finder
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:[NSArray arrayWithObjects:[NSURL fileURLWithPath:resultPath], nil]];
        NSLog(@"%@", [NSURL fileURLWithPath:resultPath].path);
    }
}

- (IBAction)shardCountChanged:(id)sender {
    
    _shardCountLabel.stringValue = _shardCountSlider.stringValue;
    _thresholdSlider.maxValue = _shardCountSlider.doubleValue;
    _thresholdSlider.numberOfTickMarks = MAX(_shardCountSlider.integerValue - 1, 2);

    [self thresholdChanged:(self.identifier)];
}

- (IBAction)thresholdChanged:(id)sender {
    _thresholdLabel.stringValue = _thresholdSlider.stringValue;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self shardCountChanged:(self.identifier)];
    [self thresholdChanged:(self.identifier)];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (void)viewDidDisappear {
    [NSApp terminate:self];
}

@end

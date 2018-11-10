// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#import "./XmlImporterPlugin.h"
#import "../plugin/xmlplugin.h"

// Our shared base class has just everything we need except the UI.  This
// connector lets us delegate the UI back to our obj-c plugin.
class CXmlImporterPluginMac : public CXmlImporterPlugin {
 public:
  CXmlImporterPluginMac(XmlImporterPlugin*& p) : plugin_(p){
  }
  void ShowOptionsDialog() {
    [plugin_ showOptionsDialog];
  }
  XmlImporterPlugin* plugin_;
};

@implementation XmlImporterPlugin

// Initialize the plugin and return an autoreleased instance
+ (NSObject<SketchUpModelImporterPlugin>*)importer {
  XmlImporterPlugin* theImporter = nil;

  theImporter = [[XmlImporterPlugin alloc] init];

  return [theImporter autorelease];
}

// This gets the underlying c++ interface.
- (SketchUpModelImporterInterface*)sketchUpModelImporterInterface {
  return _plugin;
}

- (id)init {
  self = [super init];
  if (nil == self) {
    return nil;
  }
  _plugin = new CXmlImporterPluginMac(self);
  
  // Defer load the nibs

  return self;
}

- (void)dealloc {
  delete _plugin;

  // Release the top level objects from the loaded NIB file
  if (_optionsPanel) {
    [_optionsPanel release];
  }

  [super dealloc];
}

- (bool)loadNib {
  if (!_optionsPanel) {
    [NSBundle loadNibNamed:@"OptionsDialog" owner:self];
  }
  return _optionsPanel;
}

- (void)awakeFromNib {
}

#define GET_STATE(c) ((c != nil) ? ([c state] == NSOnState) : false)
#define SET_STATE(c, b) if(c) [c setState:((b)?NSOnState:NSOffState)]

// Gathers options from the user. Only applicable if the importer supports options.
- (void)showOptionsDialog {
  if (![self loadNib]) return;

  // Preferences can be read from the plist file here.
  CXmlOptions options = _plugin->GetOptions();

  // And populate the dialog.
  SET_STATE(_mergeCoplanarFaces, options.merge_coplanar_faces());

  // Run the options dialog as a modal window.
  NSInteger response = [NSApp runModalForWindow:_optionsPanel];
  
  if (response == NSRunStoppedResponse) {
    // User pressed ok.  Collect preferences from the dialog and save them.
    options.set_merge_coplanar_faces(GET_STATE(_mergeCoplanarFaces));
    _plugin->SetOptions(options);
  }
  [_optionsPanel close];
  [_optionsPanel orderOut:self];
}

// Close the a panel started with [NSApp runModalForWindow:]
- (IBAction)closePanel:(id)sender {
  // The OK button on the options panel has a tag of 1.  While the Cancel
  // button and OK button on the report panel have a tag of zero.  All we care
  // about is that we can differentate between OK and Cancel on the options
  // dialog so we can save modified preferences.
  if ([sender tag] == 1) {
    [NSApp stopModal];
  } else { // cancel button
    [NSApp abortModal];
  }
}

@end

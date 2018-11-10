// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#import <Cocoa/Cocoa.h>
#import <SketchUpAPI/import_export/modelimporterplugin.h>

// A simple connector class that lets us connect our shared c++ plugin class to
// the UI in this obj-c class
class CXmlImporterPluginMac;

@interface XmlImporterPlugin : NSObject
    <SketchUpModelImporterPlugin, NSTextFieldDelegate>  {
  IBOutlet NSPanel* _optionsPanel;
  IBOutlet NSButton* _mergeCoplanarFaces;

  // Delegates most everything to our shared c++ plugin class.
  CXmlImporterPluginMac* _plugin;
}

// Gathers options from the user.
- (void)showOptionsDialog;

// Closes either the options panel or the summary panel.
- (IBAction)closePanel:(id)sender;

@end

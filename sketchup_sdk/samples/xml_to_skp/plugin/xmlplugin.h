// Copyright 2013 Trimble Navigation Ltd. All Rights Reserved.

#ifndef XMLTOSKP_PLUGIN_XMLPLUGIN_H
#define XMLTOSKP_PLUGIN_XMLPLUGIN_H

#include <SketchUpAPI/import_export/modelimporterplugin.h>
#include "../common/xmloptions.h"

// This is the shared portion of the xml importer plugin interface.
class CXmlImporterPlugin : public SketchUpModelImporterInterface {
 public:
  std::string GetIdentifier() const {
    // Never localized
    return "com.sketchup.importers.xml";
  }
  int GetFileExtensionCount() const {
    return 1;
  }
  std::string GetFileExtension(int index) const {
     return "xml";
  }
  std::string GetDescription() const;

  bool SupportsOptions() const {
    return true;
  }

  // The dialogs are platform dependent.
  virtual void ShowOptionsDialog() = 0;

  // The conversion is entirely platform independent!
  bool ConvertToSkp(const std::string& input_xml,
                    const std::string& output_skp,
                    SketchUpPluginProgressCallback* callback,
                    void* reserved);

  // Get-set options
  CXmlOptions GetOptions() const { return options_; }
  void SetOptions(const CXmlOptions& options) { options_ = options; }

protected:
  CXmlOptions options_;
};

#endif // XMLTOSKP_PLUGIN_XMLPLUGIN_H

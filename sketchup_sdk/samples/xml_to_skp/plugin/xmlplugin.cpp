// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include <string>
#include "./xmlplugin.h"
#include "../common/xmlimporter.h"

std::string CXmlImporterPlugin::GetDescription() const {
  return "XML Files (*.xml)";
}

bool CXmlImporterPlugin::ConvertToSkp(const std::string& input_xml,
    const std::string& output_skp,
    SketchUpPluginProgressCallback* callback,
    void* reserved) {
  // Load import preferences and set on importer
  CXmlImporter importer;
  importer.SetOptions(options_);

  // Import
  return importer.Convert(input_xml, output_skp, callback);
}

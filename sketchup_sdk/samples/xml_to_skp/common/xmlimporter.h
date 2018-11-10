// Copyright 2013 Trimble Navigation Ltd. All Rights Reserved.

#ifndef XMLTOSKP_COMMON_XMLIMPORTER_H
#define XMLTOSKP_COMMON_XMLIMPORTER_H

#include <string>
#include <map>
#include "../../common/xmlfile.h"
#include "./xmloptions.h"

#include <SketchUpAPI/model/defs.h>

class SketchUpPluginProgressCallback;

class CXmlImporter {
 public:
  CXmlImporter();
  bool Convert(const std::string& xml_in,
               const std::string& skp_out,
               SketchUpPluginProgressCallback* progress_callback);

  void SetOptions(const CXmlOptions& options) { options_ = options; }

 private:
  void ReleaseModelObjects();
  void CreateLayers(const std::vector<XmlLayerInfo>& layer_infos);
  void CreateMaterials(const std::vector<XmlMaterialInfo>& mat_infos);
  void CreateDefinitions(
      const std::vector<XmlComponentDefinitionInfo>& def_infos);
  bool CreateEntities(const XmlEntitiesInfo& info, SUEntitiesRef entities);
  SULayerRef FindLayer(const std::string& layer_name) const;
  SUMaterialRef FindMaterial(const std::string& mat_name) const;
  void BuildFaceInput(SUGeometryInputRef geom_input,
                      const XmlFaceInfo& face_info,
                      size_t num_face_vertices,
                      size_t& face_vertex_count,
                      size_t& global_vertex_count);

 private:
  // File
  CXmlFile file_;

  // Options
  CXmlOptions options_;

  // The model reference
  SUModelRef model_;

  // Name to entity maps
  std::map<std::string, SULayerRef> layers_map_;
  std::map<std::string, SUMaterialRef> materials_map_;

  // Name to component definition map
  typedef std::map<std::string,
                   std::pair<SUComponentDefinitionRef,
                             const XmlComponentDefinitionInfo*> > CompDefMap;
  CompDefMap comp_def_map_;
};

#endif // XMLTOSKP_COMMON_XMLIMPORTER_H

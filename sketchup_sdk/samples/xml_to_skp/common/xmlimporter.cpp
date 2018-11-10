// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include <string>
#include <cassert>

#include "./xmlimporter.h"
#include "../../common/utils.h"

#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/component_instance.h>
#include <SketchUpAPI/model/drawing_element.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/geometry_input.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/layer.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/texture.h>

#undef min
#include <algorithm>

using namespace XmlGeomUtils;

CXmlImporter::CXmlImporter() {
  SUSetInvalid(model_);
}

bool CXmlImporter::Convert(const std::string& xml_in,
                           const std::string& skp_out,
                           SketchUpPluginProgressCallback* progress_callback) {
  bool imported = false;
  try {
    // Initialize the SDK
    SUInitialize();

    // Open the xml file
    if (!file_.Open(xml_in, false)) {
      throw std::exception();
    }

    // Get model info from xml
    HandleProgress(progress_callback, 0.0, "Reading xml file...");
    XmlModelInfo model_info;
    imported = file_.GetModelInfo(model_info);

    // Done with the xml file
    file_.Close(false);

    if (imported) {
      // Create model
      SUSetInvalid(model_);
      SU_CALL(SUModelCreate(&model_));

      // Layers
      HandleProgress(progress_callback, 25.0, "Creating layers...");
      CreateLayers(model_info.layers_);

      // Materials
      HandleProgress(progress_callback, 50.0, "Creating materials...");
      CreateMaterials(model_info.materials_);

      // Component definitions
      HandleProgress(progress_callback, 75.0, "Creating definitions...");
      CreateDefinitions(model_info.definitions_);

      // Geometry
      SUEntitiesRef model_entities = SU_INVALID;
      SU_CALL(SUModelGetEntities(model_, &model_entities));
      CreateEntities(model_info.entities_, model_entities);

      if (options_.merge_coplanar_faces()) {
        SU_CALL(SUModelMergeCoplanarFaces(model_));
      }

      // Save the skp file
      HandleProgress(progress_callback, 100.0, "Saving...");
      SU_CALL(SUModelSaveToFile(model_, skp_out.c_str()));
    }

  } catch(...) {
    imported = false;
    file_.Close(true);
  }

  // Clean up
  ReleaseModelObjects();
  return imported;
}

void CXmlImporter::ReleaseModelObjects() {
  if (!SUIsInvalid(model_)) {
    SUModelRelease(&model_);
    SUSetInvalid(model_);
  }

  layers_map_.clear();
  materials_map_.clear();

  // Terminate the SDK
  SUTerminate();
}

void CXmlImporter::CreateLayers(const std::vector<XmlLayerInfo>& layer_infos) {

  for (std::vector<XmlLayerInfo>::const_iterator it = layer_infos.begin(),
       ite = layer_infos.end(); it != ite; ++it) {
    const XmlLayerInfo& info = *it;

    SULayerRef layer = SU_INVALID;

    const bool is_default_layer = (it == layer_infos.begin());
    if (is_default_layer) {
      // First layer is the default layer. Do not create a new one.
      size_t count = 0;
      SU_CALL(SUModelGetLayers(model_, 1, &layer, &count));
    } else {
      SU_CALL(SULayerCreate(&layer));
    }

    // Name
    SU_CALL(SULayerSetName(layer, info.name_.c_str()));

    // Material
    if (info.has_material_info_) {
      // The API does not yet support setting layer materials
    }

    // Visibility
    SU_CALL(SULayerSetVisibility(layer, info.is_visible_));

    // Add layer to the model
    if (!is_default_layer) {
      SU_CALL(SUModelAddLayers(model_, 1, &layer));
    }

    // Map to the id from the file.
    layers_map_.insert(std::make_pair(info.name_, layer));
  }
}

void CXmlImporter::CreateMaterials(
    const std::vector<XmlMaterialInfo>& mat_infos) {

  std::string texture_dir = file_.GetTextureDirectory();

  for (std::vector<XmlMaterialInfo>::const_iterator it = mat_infos.begin(),
       ite = mat_infos.end(); it != ite; ++it) {
    const XmlMaterialInfo& info = *it;
    SUMaterialRef material = SU_INVALID;
    SU_CALL(SUMaterialCreate(&material));

    // Name
    SU_CALL(SUMaterialSetName(material, info.name_.c_str()));
    
    // Color
    if (info.has_color_)
      SU_CALL(SUMaterialSetColor(material, &info.color_));

    // Opacity
    SU_CALL(SUMaterialSetUseOpacity(material, info.has_alpha_));
    if (info.has_alpha_)
      SU_CALL(SUMaterialSetOpacity(material, info.alpha_));

    // Texture
    if (info.has_texture_) {
      std::string texture_file = texture_dir + info.texture_path_;
      SUTextureRef texture = SU_INVALID;
      // Try to load the texture file.
      if (SUTextureCreateFromFile(&texture, texture_file.c_str(),
                                  info.texture_sscale_,
                                  info.texture_tscale_) == SU_ERROR_NONE) {
        SU_CALL(SUMaterialSetTexture(material, texture));
      }
    }

    // Add material to the model
    SU_CALL(SUModelAddMaterials(model_, 1, &material));

    // Map to the id from the file.
    materials_map_.insert(std::make_pair(info.name_, material));
  }
}

static SUPoint3D ConvertPoint(const CPoint3d& pt) {
  SUPoint3D su_pt = { pt.x(), pt.y(), pt.z() };
  return su_pt;
}

static SUPoint3D PointFromVertex(const XmlFaceVertex& vertex) {
  return ConvertPoint(vertex.vertex_);
}

static SUPoint2D ConvertTextureCoords(const CPoint3d& pt) {
  SUPoint2D coords = { pt.x(), pt.y() };
  return coords;
}

SULayerRef CXmlImporter::FindLayer(const std::string& layer_name) const {
  std::map<std::string, SULayerRef>::const_iterator itl =
      layers_map_.find(layer_name);
  SULayerRef layer = SU_INVALID;
  if (itl != layers_map_.end()) {
    layer = itl->second;
  }
  return layer;
}

SUMaterialRef CXmlImporter::FindMaterial(const std::string& mat_name) const {
  std::map<std::string, SUMaterialRef>::const_iterator itm =
      materials_map_.find(mat_name);
  SUMaterialRef material = SU_INVALID;
  if (itm != materials_map_.end()) {
    material = itm->second;
  }
  return material;
}

// Implementation function for CreateEntities. Given a geometry input, adds
// a face with 'num_face_vertices' vertices starting at global vertex index
// 'global_vertex_count'. Also keeps track of a running vertex count within
// the face (in case it is tessellated).
void CXmlImporter::BuildFaceInput(SUGeometryInputRef geom_input,
                                  const XmlFaceInfo& face_info,
                                  size_t num_face_vertices,
                                  size_t& face_vertex_count,
                                  size_t& global_vertex_count) {
  // Set up an outer loop input for the face
  SULoopInputRef loop = SU_INVALID;
  SU_CALL(SULoopInputCreate(&loop));
  for (size_t i = 0; i < num_face_vertices; ++i) {
    SU_CALL(SULoopInputAddVertexIndex(loop, global_vertex_count + i));
  }
  // Add the face
  size_t face_index = 0;
  SU_CALL(SUGeometryInputAddFace(geom_input, &loop, &face_index));

  // Set the layer
  if (!face_info.layer_name_.empty()) {
    SULayerRef layer = FindLayer(face_info.layer_name_);
    SU_CALL(SUGeometryInputFaceSetLayer(geom_input, face_index, layer));
  }

  // Set up the material input (Front face)
  if (!face_info.front_mat_name_.empty()) {
    SUMaterialInput mat_input = { 0 };
    mat_input.material = FindMaterial(face_info.front_mat_name_);
    if (face_info.has_front_texture_) {
      mat_input.num_uv_coords = std::min(num_face_vertices, (size_t)4);
      for (size_t i = 0; i < mat_input.num_uv_coords; ++i) {
        mat_input.vertex_indices[i] = global_vertex_count + i;
        mat_input.uv_coords[i] = ConvertTextureCoords(
            face_info.vertices_[face_vertex_count + i].front_texture_coord_);
      }
    }
    SU_CALL(SUGeometryInputFaceSetFrontMaterial(geom_input, face_index,
                                                &mat_input));
  }
  // Set up the material input (Back face)
  if (!face_info.back_mat_name_.empty()) {
    SUMaterialInput mat_input = { 0 };
    mat_input.material = FindMaterial(face_info.back_mat_name_);
    if (face_info.has_back_texture_) {
      mat_input.num_uv_coords = std::min(num_face_vertices, (size_t)4);
      for (size_t i = 0; i < mat_input.num_uv_coords; ++i) {
        mat_input.vertex_indices[i] = global_vertex_count + i;
        mat_input.uv_coords[i] = ConvertTextureCoords(
            face_info.vertices_[face_vertex_count + i].back_texture_coord_);
      }
    }
    SU_CALL(SUGeometryInputFaceSetBackMaterial(geom_input, face_index,
                                               &mat_input));
  }
  face_vertex_count+= num_face_vertices;
  global_vertex_count += num_face_vertices;
};

void CXmlImporter::CreateDefinitions(
    const std::vector<XmlComponentDefinitionInfo>& def_infos) {
  for (std::vector<XmlComponentDefinitionInfo>::const_iterator it =
       def_infos.begin(), ite = def_infos.end(); it != ite; ++it) {
    SUComponentDefinitionRef definition = SU_INVALID;
    SU_CALL(SUComponentDefinitionCreate(&definition));
    SU_CALL(SUComponentDefinitionSetName(definition, it->name_.c_str()));
    // Add definition to model before making any other operation on it.
    SU_CALL(SUModelAddComponentDefinitions(model_, 1, &definition));
    comp_def_map_.insert(std::make_pair(it->name_,
                                        std::make_pair(definition, &(*it))));
  }
}

bool CXmlImporter::CreateEntities(const XmlEntitiesInfo& info,
                                  SUEntitiesRef entities) {
  bool ok = true;

  // Component instances
  for (std::vector<XmlComponentInstanceInfo>::const_iterator it =
       info.component_instances_.begin(), ite = info.component_instances_.end();
       it != ite; ++it) {
    const XmlComponentInstanceInfo& instance_info = *it;
    // Find the definition
    CompDefMap::const_iterator itdef =
        comp_def_map_.find(instance_info.definition_name_);
    if (itdef == comp_def_map_.end()) {
      ok = false;
      continue;
    }
    SUComponentDefinitionRef definition = itdef->second.first;
    const XmlComponentDefinitionInfo* def_info = itdef->second.second;
    // Create the instance
    SUComponentInstanceRef instance = SU_INVALID;
    SU_CALL(SUComponentDefinitionCreateInstance(definition, &instance));
    // Add the instance to the parent entities
    SU_CALL(SUEntitiesAddInstance(entities, instance, NULL));
    // Set material and layer if set
    if (!instance_info.layer_name_.empty()) {
      SULayerRef layer = FindLayer(instance_info.layer_name_);
      SU_CALL(SUDrawingElementSetLayer(
              SUComponentInstanceToDrawingElement(instance), layer));
    }
    if (!instance_info.material_name_.empty()) {
      SUMaterialRef mat = FindMaterial(instance_info.material_name_);
      SU_CALL(SUDrawingElementSetMaterial(
              SUComponentInstanceToDrawingElement(instance), mat));
    }
    // Set the transformation
    SU_CALL(SUComponentInstanceSetTransform(instance,
                                            &instance_info.transform_));
    // Populate the entities of the definition using recursion
    SUEntitiesRef instance_entities = SU_INVALID;
    SU_CALL(SUComponentDefinitionGetEntities(definition, &instance_entities));
    ok &= CreateEntities(def_info->entities_, instance_entities);
  }

  // Groups
  for (std::vector<XmlGroupInfo>::const_iterator it = info.groups_.begin(),
       ite = info.groups_.end(); it != ite; ++it) {
    const XmlGroupInfo& group_info = *it;
    // Create a group
    SUGroupRef group = SU_INVALID;
    SU_CALL(SUGroupCreate(&group));
    // Set the transformation
    SU_CALL(SUGroupSetTransform(group, &group_info.transform_));
    // Add the group to the parent entities
    SU_CALL(SUEntitiesAddGroup(entities, group));
    // Populate the entities of the group using recursion
    SUEntitiesRef group_entities = SU_INVALID;
    SU_CALL(SUGroupGetEntities(group, &group_entities));
    ok &= CreateEntities(*group_info.entities_, group_entities);
  }

  // Create faces
  SUGeometryInputRef geom_input = SU_INVALID;
  SU_CALL(SUGeometryInputCreate(&geom_input));

  size_t global_vertex_count = 0;
  for (std::vector<XmlFaceInfo>::const_iterator it = info.faces_.begin(),
       ite = info.faces_.end(); it != ite; ++it) {
    const XmlFaceInfo& face_info = *it;
    // Add vertices
    for (std::vector<XmlFaceVertex>::const_iterator it =
         face_info.vertices_.begin(), ite = face_info.vertices_.end();
         it != ite; ++it) {
      SUPoint3D pt = PointFromVertex(*it);
      SU_CALL(SUGeometryInputAddVertex(geom_input, &pt));
    }
    const size_t num_vertices = face_info.vertices_.size();
    if (face_info.has_single_loop_) {
      // Face has an outer loop only.
      size_t face_vertex_count = 0;
      BuildFaceInput(geom_input, face_info, num_vertices, face_vertex_count,
                     global_vertex_count);
    } else {
      // The face was tessellated into a triangular mesh.
      assert(num_vertices % 3 == 0);
      const size_t num_triangles = num_vertices / 3;
      size_t face_vertex_count = 0;
      for (size_t tri_index = 0; tri_index < num_triangles; ++tri_index) {
        BuildFaceInput(geom_input, face_info, 3, face_vertex_count,
                       global_vertex_count);
      }
    }
  }

  // Fill the entities
  ok &= SUEntitiesFill(entities, geom_input, true) == SU_ERROR_NONE;
  // Clean up geom_input
  SU_CALL(SUGeometryInputRelease(&geom_input));

  // Create stand-alone edges
  if (!info.edges_.empty()) {
    for (std::vector<XmlEdgeInfo>::const_iterator it = info.edges_.begin(),
          ite = info.edges_.end(); it != ite; ++it) {
      const XmlEdgeInfo& edge_info = *it;
      SUPoint3D start_pt = ConvertPoint(edge_info.start_);
      SUPoint3D end_pt = ConvertPoint(edge_info.end_);
      // Create an edge
      SUEdgeRef edge = SU_INVALID;
      SU_CALL(SUEdgeCreate(&edge, &start_pt, &end_pt));
      // Color
      if (edge_info.has_color_) {
        SU_CALL(SUEdgeSetColor(edge, &edge_info.color_));
      }
      // Layer
      if (edge_info.has_layer_) {
        SULayerRef layer = FindLayer(edge_info.layer_name_);
        SU_CALL(SUDrawingElementSetLayer(SUEdgeToDrawingElement(edge), layer));
      }
      // Add to the entities
      SU_CALL(SUEntitiesAddEdges(entities, 1, &edge));
    }
  }


  return ok;
}

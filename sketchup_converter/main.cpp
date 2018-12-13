//
//  main.cpp
//  sketchup_converter
//
//  Created by jahsia on 10/26/18.
//  Copyright © 2018 trisetra. All rights reserved.
//

#include <iostream>

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/geometry/vector3d.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/component_instance.h>
#include <SketchUpAPI/model/drawing_element.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/entity.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/image_rep.h>
#include <SketchUpAPI/model/layer.h>
#include <SketchUpAPI/model/loop.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/mesh_helper.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/texture.h>
#include <SketchUpAPI/model/texture_writer.h>
#include <SketchUpAPI/model/uv_helper.h>
#include <SketchUpAPI/model/vertex.h>
#include <SketchUpAPI/unicodestring.h>
#include <Eigen/Dense>
#include "MeshImporter.hpp"
#include <map>
#include <vector>
#include <unordered_set>
#define SU_CALL(func)          \
if ((func) != SU_ERROR_NONE) \
throw std::exception()

#define Y_UP false
using namespace trisetra;

  class CSUString {
  public:
    CSUString() {
      SUSetInvalid(su_str_);
      SUStringCreate(&su_str_);
    }
    
    ~CSUString() {
      SUStringRelease(&su_str_);
    }
    
    operator SUStringRef*() {
      return &su_str_;
    }
    
    std::string utf8() {
      size_t length;
      SUStringGetUTF8Length(su_str_, &length);
      std::string string;
      string.resize(length + 1);
      size_t returned_length;
      SUStringGetUTF8(su_str_, length, &string[0], &returned_length);
      string.erase(length, 1);
      return string;
    }
    
  private:
    // Disallow copying for simplicity
    CSUString(const CSUString& copy);
    CSUString& operator=(const CSUString& copy);
    
    SUStringRef su_str_;
  };
  
  static std::string GetComponentDefinitionName(SUComponentDefinitionRef comp_def) {
    CSUString name;
    SU_CALL(SUComponentDefinitionGetName(comp_def, name));
    return name.utf8();
  }
  
  struct SUImportInfo {
    std::vector<SUMaterialRef>                  mats;
    std::vector<std::string>                    names;
    std::vector<SUImageRepRef>                     textures;
    std::vector<SUPoint2D>                      texST;
    std::map<void*, MeshSource*>      def_map;
    std::map<void*, std::vector<SUMaterialRef>> mat_map;
  };
  
  struct SUPolyInfo {
    std::vector<float>    vertex_positions;  // The vertex's position (x,y,z)
    std::vector<uint32_t> vertex_indices;
    std::vector<float>    vertex_normals;  // The vertex's surface normal (x,y,z)
    std::vector<float>    uvs;             // The vertex's texture coordinates (u,v)
    std::vector<int32_t>  material_ids;
    std::vector<int32_t>  face_material;
  };
  
  static std::vector<float> EigenToVector(const Eigen::Affine3f& t) {
    std::vector<float> local_transformations(12);
    local_transformations[0] = t(0, 0);
    local_transformations[1] = t(1, 0);
    local_transformations[2] = t(2, 0);
    
    local_transformations[3] = t(0, 1);
    local_transformations[4] = t(1, 1);
    local_transformations[5] = t(2, 1);
    local_transformations[6] = t(0, 2);
    local_transformations[7] = t(1, 2);
    local_transformations[8] = t(2, 2);
    
    local_transformations[9] = t(0, 3);
    local_transformations[10] = t(1, 3);
    local_transformations[11] = t(2, 3);
    
    return local_transformations;
  }
  
  static Eigen::Affine3f ToEigenAffine(const double* raw_data) {
    Eigen::Affine3f t;
    float           scale = 1.0f / (float)(raw_data[15]);
    t(0, 0) = (float)(raw_data[0]) * scale;
    t(1, 0) = (float)(raw_data[1]) * scale;
    t(2, 0) = (float)(raw_data[2]) * scale;
    t(3, 0) = 0.0f;
    t(0, 1) = (float)(raw_data[4]) * scale;
    t(1, 1) = (float)(raw_data[5]) * scale;
    t(2, 1) = (float)(raw_data[6]) * scale;
    t(3, 1) = 0.0f;
    t(0, 2) = (float)(raw_data[8]) * scale;
    t(1, 2) = (float)(raw_data[9]) * scale;
    t(2, 2) = (float)(raw_data[10]) * scale;
    t(3, 2) = 0.0f;
    t(0, 3) = (float)(raw_data[12]) * scale;
    t(1, 3) = (float)(raw_data[13]) * scale;
    t(2, 3) = (float)(raw_data[14]) * scale;
    t(3, 3) = 1.0f;
    
    return t;
  }
  

  static void InitMaterialData(std::shared_ptr<MaterialData>& material, const SUImportInfo& su_mats, int src_idx) {
    SUColor color;
    double  opt = 1.0;
    bool    use_opacity;
    SUMaterialGetColor(su_mats.mats[src_idx], &color);
    SUMaterialGetOpacity(su_mats.mats[src_idx], &opt);
    SUMaterialGetUseOpacity(su_mats.mats[src_idx], &use_opacity);
    material->name = su_mats.names[src_idx];
    material->base_color[0] = color.red / 255.0f;
    material->base_color[1] = color.green / 255.0f;
    material->base_color[2] = color.blue / 255.0f;
    material->base_color[3] = color.alpha / 255.0f;
    if (use_opacity)
      material->opacity = opt;
  }
  static void AddTextrueToMat(SUImageRepRef&                         img_rep,
                              std::shared_ptr<MaterialData>& material,
                              const std::string&                     composed_name) {
    if (SUIsValid(img_rep)) {
      size_t width, height, data_size, bits_per_pixel;
      SUImageRepGetPixelDimensions(img_rep, &width, &height);
      SUImageRepGetDataSize(img_rep, &data_size, &bits_per_pixel);
      std::string file_path = "./" +composed_name+".png";
      SUImageRepSaveToFile(img_rep, file_path.c_str());
      material->base_color_map = file_path;
    }
  }
  
  // returns first the transform to store for the node3d
  // the second transform (might cointains negtive scale) to be baked into vertecies
  static std::tuple<Eigen::Affine3f, Eigen::Affine3f> DecomposeTransform(Eigen::Affine3f t) {
    Eigen::Affine3f original_t = t;
    
    Eigen::Matrix3f rotation_scale_part(original_t.linear());
    if (rotation_scale_part.determinant() > 0.0f) {
      Eigen::Affine3f identity = Eigen::Affine3f::Identity();
      return std::make_tuple(original_t, identity);
    } else {
      t.translation() = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
      
      Eigen::Matrix3f M = t.linear();
      Eigen::Matrix3f rotation_matrix = t.rotation();
      Eigen::Matrix3f scaling_matrix = rotation_matrix.transpose() * M;
      
      // sanitize non rotational component (only scaling > 0)
      scaling_matrix(0, 0) = std::abs(scaling_matrix(0, 0));
      scaling_matrix(1, 1) = std::abs(scaling_matrix(1, 1));
      scaling_matrix(2, 2) = std::abs(scaling_matrix(2, 2));
      scaling_matrix(0, 1) = scaling_matrix(0, 2) = scaling_matrix(1, 0) = scaling_matrix(1, 2) = scaling_matrix(2, 0) =
      scaling_matrix(2, 1) = 0.0f;
      t.linear() = rotation_matrix * scaling_matrix;
      t.translation() = original_t.translation();
      Eigen::Affine3f inverse_t = t.inverse();
      Eigen::Affine3f bake_t = inverse_t * original_t;
      
      return std::make_tuple(t, bake_t);
    }
  }
  
  static void WriteFace(SUFaceRef              face,
                        SUTextureWriterRef     texture_writer,
                        SUPolyInfo&            m_data,
                        SUImportInfo&          mat_info,
                        int                    parent_idx,
                        bool                   front,
                        SUMaterialRef          assigned,
                        bool                   instanced,
                        MeshImport*  mesh_import,
                        MeshSource*  mesh,
                        const Eigen::Affine3f& transform,
                        const Eigen::Matrix3f& normal_transform) {
    if (SUIsInvalid(face))
      return;
    
    SUMaterialRef face_material = SU_INVALID;
    
    // sort of hack as may face expect back face to be front facing due to material assignment.
    // so we use a key here to be smart about back/front face selection
    bool prefer_back_face = false;
    bool has_face_material = true;
    {
      SUMaterialRef bface_material = SU_INVALID;
      SUTextureRef  ftexture_ref = SU_INVALID;
      SUFaceGetFrontMaterial(face, &face_material);
      if (SUIsValid(face_material)) {
        SUMaterialGetTexture(face_material, &ftexture_ref);
      }
      SUFaceGetBackMaterial(face, &bface_material);
      
      if (SUIsValid(bface_material)) {
        SUTextureRef texture_ref = SU_INVALID;
        SUMaterialGetTexture(bface_material, &texture_ref);
        if (SUIsValid(texture_ref) && SUIsInvalid(ftexture_ref)) {
          SUFaceReverse(face);
          face_material = bface_material;
          prefer_back_face = true;
        }
      }
    }
    
    SUVector3D fnorm;
    SUFaceGetNormal(face, &fnorm);
    // Get the current front and back materials off of our stack
    
    if (SUIsInvalid(face_material)) {
      face_material = assigned;
      has_face_material = false;
    }
    
    long mat_idx = 0;
    
    auto find_mat_it = std::find_if(
                                    mat_info.mats.begin(), mat_info.mats.end(), [face_material](const SUMaterialRef& in) { return face_material.ptr == in.ptr; });
    if (find_mat_it != mat_info.mats.end()) {
      mat_idx = find_mat_it - mat_info.mats.begin();
    }
    
    bool has_texture = SUIsValid(mat_info.textures[mat_idx]);  // info.has_front_texture_ || info.has_back_texture_;
    
    if (has_texture && has_face_material) {
      long textureId = 0;
      SUTextureWriterGetTextureIdForFace(texture_writer, face, true, &textureId);
    }
    
    size_t local_mat_id = std::distance(m_data.material_ids.begin(), find(m_data.material_ids.begin(), m_data.material_ids.end(), mat_idx));
    
    if (local_mat_id == m_data.material_ids.size()) {
      m_data.material_ids.push_back((int)mat_idx);
    }
    
    if (mesh) {
      // 0 is defefault material
      auto eu_material = mesh_import->get_material((int)mat_idx);
      mesh_import->apply_material(mesh, eu_material.get());
    }
    
    
    // Get a uv helper
    float inv_ss = 1.0f;
    float inv_tt = 1.0f;
    
    if (!has_face_material) {
      inv_ss = mat_info.texST[mat_idx].x;
      inv_tt = mat_info.texST[mat_idx].y;
    }
    // Find out how many loops the face has
    size_t num_loops = 0;
    SU_CALL(SUFaceGetNumInnerLoops(face, &num_loops));
    num_loops++;  // add the outer loop
    
    // If this is a complex face with one or more holes in it
    // we tessellate it into triangles using the polygon mesh class, then
    // export each triangle as a face.
    // info.has_single_loop_ = false;
    
    // Create a mesh from face.
    SUMeshHelperRef mesh_ref = SU_INVALID;
    SU_CALL(SUMeshHelperCreateWithTextureWriter(&mesh_ref, face, texture_writer));
    
    // Get the vertices
    size_t num_vertices = 0;
    SU_CALL(SUMeshHelperGetNumVertices(mesh_ref, &num_vertices));
    if (num_vertices == 0)
      return;
    
    std::vector<SUPoint3D> vertices(num_vertices);
    SU_CALL(SUMeshHelperGetVertices(mesh_ref, num_vertices, &vertices[0], &num_vertices));
    
    std::vector<SUVector3D> normals(num_vertices);
    SU_CALL(SUMeshHelperGetNormals(mesh_ref, num_vertices, &normals[0], &num_vertices));
    std::vector<SUPoint3D> stq_coords(num_vertices);
    size_t                 uv_size = 0;
    if (!prefer_back_face) {
      SUMeshHelperGetFrontSTQCoords(mesh_ref, num_vertices, &stq_coords[0], &uv_size);
    } else {
      SUMeshHelperGetBackSTQCoords(mesh_ref, num_vertices, &stq_coords[0], &uv_size);
    }
    
    std::vector<float> vertex_positions(num_vertices * 3);
    std::vector<float> vertex_normal(num_vertices * 3);
    
    // dimension is Y up
    for (size_t i = 0; i < num_vertices; ++i) {
      Eigen::Vector3f pos(vertices[i].x, vertices[i].y, vertices[i].z);
      Eigen::Vector3f norm(normals[i].x, normals[i].y, normals[i].z);
      Eigen::Vector3f post_trans = transform * pos;
      Eigen::Vector3f norm_trans = normal_transform * norm;
      norm_trans.normalize();
      
      vertex_positions[i * 3 + 0] = post_trans.x();
      vertex_positions[i * 3 + 1] = post_trans.y();
      vertex_positions[i * 3 + 2] = post_trans.z();
      vertex_normal[i * 3 + 0] = norm_trans.x();
      vertex_normal[i * 3 + 1] = norm_trans.y();
      vertex_normal[i * 3 + 2] = norm_trans.z();
    }
    
    // Get triangle indices.
    size_t num_triangles = 0;
    SU_CALL(SUMeshHelperGetNumTriangles(mesh_ref, &num_triangles));
    const size_t        num_indices = 3 * num_triangles;
    size_t              num_retrieved = 0;
    std::vector<size_t> indices(num_indices);
    SU_CALL(SUMeshHelperGetVertexIndices(mesh_ref, num_indices, &indices[0], &num_retrieved));
    std::vector<uint32_t> dst_idx(num_indices);
    
    std::vector<float> uv_coord;
    if (uv_size > 0) {
      uv_coord.resize(num_vertices * 2);
      for (size_t i = 0; i < num_vertices; ++i) {
        uv_coord[i * 2 + 0] = inv_ss * stq_coords[i].x / stq_coords[i].z;
        uv_coord[i * 2 + 1] = inv_tt * stq_coords[i].y / stq_coords[i].z;
      }
    }
    
    size_t               base_idx = m_data.vertex_positions.size() / 3;
    std::vector<int32_t> face_materials(num_triangles);
    
    front = !prefer_back_face;
    for (size_t i_triangle = 0; i_triangle < num_triangles; i_triangle++) {
      face_materials[i_triangle] = (int)local_mat_id;
      // Three points in each triangle
      if (front) {
        for (size_t i = 0; i < 3; i++) {
          size_t index = indices[i_triangle * 3 + i];
          dst_idx[i_triangle * 3 + i] = (int)(index + base_idx);
        }
      } else {
        // back face
        size_t index0 = indices[i_triangle * 3 + 0];
        size_t index1 = indices[i_triangle * 3 + 1];
        size_t index2 = indices[i_triangle * 3 + 2];
        dst_idx[i_triangle * 3 + 0] = (int)(index2 + base_idx);
        dst_idx[i_triangle * 3 + 1] = (int)(index1 + base_idx);
        dst_idx[i_triangle * 3 + 2] = (int)(index0 + base_idx);
      }
    }
    // combine to original array
    
    m_data.vertex_positions.insert(m_data.vertex_positions.end(), vertex_positions.begin(), vertex_positions.end());
    m_data.vertex_normals.insert(m_data.vertex_normals.end(), vertex_normal.begin(), vertex_normal.end());
    m_data.vertex_indices.insert(m_data.vertex_indices.end(), dst_idx.begin(), dst_idx.end());
    m_data.face_material.insert(m_data.face_material.end(), face_materials.begin(), face_materials.end());
    
    if (uv_size > 0) {
      m_data.uvs.insert(m_data.uvs.end(), uv_coord.begin(), uv_coord.end());
    }
  }
  
  static void WriteEntities(SUEntitiesRef         entities,
                            SUTextureWriterRef    texture_writer,
                            SUGroupRef            group,
                            SUImportInfo&         mat_info,
                            int                   parent_idx,
                            SUMaterialRef         parent_mat,
                            MeshImport* mesh_import,
                            const Node* parent,
                            Eigen::Affine3f       bake_transform) {
#if 1
    size_t num_instances = 0;
    SU_CALL(SUEntitiesGetNumInstances(entities, &num_instances));
    if (num_instances > 0) {
      std::vector<SUComponentInstanceRef> instances(num_instances);
      SU_CALL(SUEntitiesGetInstances(entities, num_instances, &instances[0], &num_instances));
      for (size_t c = 0; c < num_instances; c++) {
        SUComponentInstanceRef   instance = instances[c];
        SUComponentDefinitionRef definition = SU_INVALID;
        SU_CALL(SUComponentInstanceGetDefinition(instance, &definition));
        
        size_t attached_instance = 0;
        SUComponentInstanceGetNumAttachedInstances(instance, &attached_instance);
        
        SUEntitiesRef entity_from_def;
        SUComponentDefinitionGetEntities(definition, &entity_from_def);
        size_t num_faces;
        SUEntitiesGetNumFaces(entity_from_def, &num_faces);
        
        SUTransformation transform;
        SU_CALL(SUComponentInstanceGetTransform(instance, &transform));
        const double* raw_data = (const double*)transform.values;
        //---- checking if matrix has neg scal term, if so, we need to bake into vertecies
        auto            src_affine = ToEigenAffine(raw_data);
        auto            stacked = src_affine;
        Eigen::Affine3f to_transform;
        Eigen::Affine3f to_bake;
        std::tie(to_transform, to_bake) = DecomposeTransform(src_affine);
        bool need_baking = !to_bake.matrix().isIdentity();
        auto sanitized_transform = EigenToVector(bake_transform * to_transform);
        //-----------------------------------------------------------------------------
        
        std::string def_name = GetComponentDefinitionName(definition);
        
        // add transformation info
        auto instance_node = mesh_import->create_node(parent, def_name);
        mesh_import->add_tranform3x4(instance_node.get(), std::move(sanitized_transform));
        
        SUMaterialRef material = SU_INVALID;
        SUDrawingElementGetMaterial(SUComponentInstanceToDrawingElement(instance), &material);
        if (num_faces > 0) {
          MeshSource* mesh_node = nullptr;
          auto                  eu_mesh = mat_info.def_map.find(definition.ptr);
          if ((eu_mesh == mat_info.def_map.end()) || need_baking) {
            auto mesh = mesh_import->create_mesh(def_name);
            // only sotre into map if we dont need baking
            if (!need_baking)
              mat_info.def_map[definition.ptr] = mesh.get();
            mesh_node = mesh.get();
            mesh_import->add_face_descriptor(mesh_node, {3});
            mesh_import->add_mesh_to_node(instance_node.get(), mesh_node);
          } else {
            // hook up instance.
            mesh_import->add_mesh_to_node(instance_node.get(), eu_mesh->second);
          }
          
          std::vector<SUFaceRef> faces(num_faces);
          SU_CALL(SUEntitiesGetFaces(entity_from_def, num_faces, &faces[0], &num_faces));
          
          if (SUIsValid(material) == false) {
            material = parent_mat;
          }
          
          SUPolyInfo front_mesh;
          auto       normal_transform = ((to_bake.linear()).inverse()).transpose();
          for (size_t i = 0; i < num_faces; i++) {
            WriteFace(faces[i],
                      texture_writer,
                      front_mesh,
                      mat_info,
                      parent_idx,
                      true,
                      material,
                      true,
                      mesh_import,
                      mesh_node,
                      to_bake,
                      normal_transform);
          }
          
          if (mesh_node) {
            mesh_import->add_positions(mesh_node, std::move(front_mesh.vertex_positions), std::move(front_mesh.vertex_indices));
            mesh_import->add_normals(mesh_node, std::move(front_mesh.vertex_normals), {});
            mesh_import->add_uv(mesh_node, 0, std::move(front_mesh.uvs), {});
            mesh_import->add_face_material_idx(mesh_node, std::move(front_mesh.face_material));
          }
        }
        
        WriteEntities(entity_from_def, texture_writer, group, mat_info, -1, material, mesh_import, instance_node.get(), to_bake);
      }
    }
#endif
    // Groups
    size_t num_groups = 0;
    SU_CALL(SUEntitiesGetNumGroups(entities, &num_groups));
    if (num_groups > 0) {
      std::vector<SUGroupRef> groups(num_groups);
      SU_CALL(SUEntitiesGetGroups(entities, num_groups, &groups[0], &num_groups));
      for (size_t g = 0; g < num_groups; g++) {
        SUGroupRef group = groups[g];
        if (!SUIsValid(group)) {
          continue;
        }
        // SUComponentDefinitionRef group_component = SU_INVALID;
        SUEntitiesRef group_entities = SU_INVALID;
        SU_CALL(SUGroupGetEntities(group, &group_entities));
        
        int              my_idx = parent_idx;
        size_t           num_faces = 0;
        SUTransformation transform;
        SU_CALL(SUGroupGetTransform(group, &transform));
        // FbxAMatrix& lLocalTransform = pNode->EvaluateLocalTransform();
        const double* raw_data = (const double*)transform.values;
        
        //---- checking if matrix has neg scal term, if so, we need to bake into vertecies
        auto            src_affine = ToEigenAffine(raw_data);
        auto            stacked = src_affine;
        Eigen::Affine3f to_transform;
        Eigen::Affine3f to_bake;
        std::tie(to_transform, to_bake) = DecomposeTransform(src_affine);
        auto sanitized_transform = EigenToVector(bake_transform * to_transform);
        //-----------------------------------------------------------------------------
        
        CSUString name;
        SUGroupGetName(group, name);
        
        auto def_name = "group_" + name.utf8();
        //------ add transformation info
        auto instance_node = mesh_import->create_node(parent, def_name);
        mesh_import->add_tranform3x4(instance_node.get(), std::move(sanitized_transform));
        
        SU_CALL(SUEntitiesGetNumFaces(group_entities, &num_faces));
        SUDrawingElementRef dwrf = SUGroupToDrawingElement(group);
        SUMaterialRef       material = SU_INVALID;
        
        if (SUIsValid(dwrf)) {
          SUDrawingElementGetMaterial(dwrf, &material);
          if (SUIsValid(material))
            parent_mat = material;
        }
        
        if (num_faces > 0) {
          auto mesh = mesh_import->create_mesh(name.utf8());
          mesh_import->add_face_descriptor(mesh.get(), {3});
          mesh_import->add_mesh_to_node(instance_node.get(), mesh.get());
          
          std::vector<SUFaceRef> faces(num_faces);
          SU_CALL(SUEntitiesGetFaces(group_entities, num_faces, &faces[0], &num_faces));
          if (SUIsInvalid(material))
            material = parent_mat;
          SUPolyInfo front_mesh;
          auto       normal_transform = ((to_bake.linear()).inverse()).transpose();
          for (size_t i = 0; i < num_faces; i++) {
            WriteFace(faces[i],
                      texture_writer,
                      front_mesh,
                      mat_info,
                      parent_idx,
                      true,
                      material,
                      false,
                      mesh_import,
                      mesh.get(),
                      to_bake,
                      normal_transform);
          }
          
          if (mesh.get()) {
            mesh_import->add_positions(mesh.get(), std::move(front_mesh.vertex_positions), std::move(front_mesh.vertex_indices));
            mesh_import->add_normals(mesh.get(), std::move(front_mesh.vertex_normals), {});
            mesh_import->add_uv(mesh.get(), 0, std::move(front_mesh.uvs), {});
            mesh_import->add_face_material_idx(mesh.get(), std::move(front_mesh.face_material));
          }
        }
        
        // Write entities
        WriteEntities(group_entities, texture_writer, group, mat_info, my_idx, parent_mat, mesh_import, instance_node.get(), to_bake);
      }
    }
  }
  
  void load_skp(const std::string& path, MeshImport* mesh_import) {
    // Always initialize the API before using it
    SUInitialize();
    
    // Load the model from a file
    SUModelRef model = SU_INVALID;
    SUResult   res = SUModelCreateFromFile(&model, path.c_str());
    
    // It's best to always check the return code from each SU function call.
    // Only showing this check once to keep this example short.
    if (res != SU_ERROR_NONE)
      throw std::runtime_error("SUModelCreateFromFile failed to open: " + path);
    
    SUTextureWriterRef texture_writer;
    SU_CALL(SUTextureWriterCreate(&texture_writer));
    
    // Get the entity container of the model.
    SUEntitiesRef entities = SU_INVALID;
    SUModelGetEntities(model, &entities);
    
    //---------------- material ------------------------
    size_t material_count = 0;
    SUModelGetNumMaterials(model, &material_count);
    
    SUImportInfo su_mats;
    su_mats.mats.resize(material_count + 1);
    su_mats.textures.resize(material_count + 1);
    su_mats.names.resize(material_count + 1);
    su_mats.texST.resize(material_count + 1);
    
    // with default material
    std::vector<std::shared_ptr<MaterialData>> materials(material_count + 1);
    ;
    for (int i = 0; i < material_count + 1; ++i) {
      materials[i] = std::make_shared<MaterialData>();
    }
    // material creation...
    {
      SUModelGetMaterials(model, material_count, &su_mats.mats[1], &material_count);
      // set 0 to default material
      // su_mats.names[0] = "default";
      materials[0]->name = "default";
      su_mats.textures[0] = SU_INVALID;
      su_mats.mats[0] = SU_INVALID;
      for (int i = 1; i < material_count + 1; ++i) {
        CSUString name;
        SUMaterialGetNameLegacyBehavior(su_mats.mats[i], name);
        SUTextureRef texture_ref = SU_INVALID;
        SUMaterialGetTexture(su_mats.mats[i], &texture_ref);
        su_mats.names[i] = name.utf8();
        
        InitMaterialData(materials[i], su_mats, i);
        
        if (SUIsValid(texture_ref)) {
          size_t width, height;
          double ss, st;
          SUTextureGetDimensions(texture_ref, &width, &height, &ss, &st);
          SUImageRepRef img_rep = SU_INVALID;
          SUImageRepCreate(&img_rep);
          su_mats.textures[i] = img_rep;
          SUTextureGetImageRep(texture_ref, &img_rep);
          CSUString tex_name;
          SUTextureGetFileName(texture_ref, tex_name);
          std::string composed_name = tex_name.utf8() + "_" + name.utf8();
          su_mats.texST[i].x = (float)ss;
          su_mats.texST[i].y = (float)st;
          AddTextrueToMat(img_rep, materials[i], composed_name );
          SUImageRepRelease(&img_rep);
        }
      }
    }
    
    mesh_import->add_materials(materials);
    
    // Get model name
    CSUString name;
    SUModelGetName(model, name);
    
    std::vector<float> local_transformations(12);
    {
      local_transformations[0] = 1;
      local_transformations[1] = 0;
      local_transformations[2] = 0;
      local_transformations[3] = 0;
      local_transformations[4] = 1;
      local_transformations[5] = 0;
      local_transformations[6] = 0;
      local_transformations[7] = 0;
      local_transformations[8] = 1;
      local_transformations[9] = 0;
      local_transformations[10] = 0;
      local_transformations[11] = 0;
    }
    
    auto root_node = mesh_import->create_node(nullptr, "y_up_convert" + name.utf8());
    { mesh_import->add_tranform3x4(root_node.get(), std::move(local_transformations)); }
    
    size_t          num_faces = 0;
    Eigen::Affine3f identity;
    identity.setIdentity();
    SU_CALL(SUEntitiesGetNumFaces(entities, &num_faces));
    if (num_faces > 0) {
      auto en_mesh = mesh_import->create_mesh("entity");
      mesh_import->add_face_descriptor(en_mesh.get(), {3});
      mesh_import->add_mesh_to_node(root_node.get(), en_mesh.get());
      
      std::vector<SUFaceRef> faces(num_faces);
      SU_CALL(SUEntitiesGetFaces(entities, num_faces, &faces[0], &num_faces));
      
      SUPolyInfo front_mesh;
      for (size_t i = 0; i < num_faces; i++) {
        WriteFace(faces[i],
                  texture_writer,
                  front_mesh,
                  su_mats,
                  0,
                  true,
                  SU_INVALID,
                  false,
                  mesh_import,
                  en_mesh.get(),
                  identity,
                  identity.linear());
      }
      
      if (en_mesh.get()) {
        mesh_import->add_positions(en_mesh.get(), std::move(front_mesh.vertex_positions), std::move(front_mesh.vertex_indices));
        mesh_import->add_normals(en_mesh.get(), std::move(front_mesh.vertex_normals), {});
        mesh_import->add_uv(en_mesh.get(), 0, std::move(front_mesh.uvs), {});
        mesh_import->add_face_material_idx(en_mesh.get(), std::move(front_mesh.face_material));
      }
    }
    
    // material gathering
    // GetEntitiesMaterial(entities, texture_writer, SU_INVALID, model_data_ptr, su_mats, 0);
    
    // Groups
    SUMaterialRef material = SU_INVALID;
    WriteEntities(entities, texture_writer, SU_INVALID, su_mats, 0, material, mesh_import, root_node.get(), identity);
    
    // Must release the model or there will be memory leaks
    SUModelRelease(&model);
    
    // Always terminate the API when done using it
    SUTerminate();
  }


int main(int argc, const char * argv[]) {
  // insert code here...
  MeshImporter mi;
  if( argc > 1){
    float rotate = 0.0f;
    std::string file_name = std::string(argv[1]);
    if(argc > 2)
      rotate = std::stof( std::string(argv[2]));
    load_skp(file_name, &mi );
    
    size_t lastindex = file_name.find_last_of(".");
    std::string rawname = file_name.substr(0, lastindex);
    rawname = rawname + ".tri";
    //mi.serialize_to_file(rawname, true, Y_UP, -1.571f);
    mi.serialize_to_file(rawname, true, Y_UP, rotate);
  }
  
  return 0;
}

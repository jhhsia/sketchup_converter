//
//  MeshImporter.hpp
//  sketchup_converter
//
//  Created by jahsia on 11/8/18.
//  Copyright © 2018 trisetra. All rights reserved.
//

#ifndef MeshImporter_hpp
#define MeshImporter_hpp
#include <iostream>

#include "MeshImport.h"
#include <stdio.h>
using namespace trisetra;
class MeshImporter : public MeshImport{
public:
  MeshImporter() = default;
  ~MeshImporter() = default;
  
  ////////////// Mesh Operations //////////////
  void add_positions(MeshSource* mesh, std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) override;
  void add_normals(MeshSource* mesh,std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) override;
  void add_tangent(MeshSource* mesh,std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) override;
  void add_binormal(MeshSource* mesh,std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) override;
  //  idx of channel. 0,1,2,3 = RGBA, 0,1,2,-1 = RGB. specify -1 to ignore channel
  void add_color(MeshSource* mesh,int r_idx, int g_idx, int b_idx, int a_idx, std::vector<float>&& src , std::vector<uint32_t>&& idx_buffer) override;
  void add_uv(MeshSource* mesh, uint32_t idx, std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) override;
  
  // indicate number of vertcies per face
  void add_face_descriptor(MeshSource* mesh,std::vector<uint8_t>&& vert_per_face) override;
  // material_idx is used to access add_materials std::vector<std::shared_ptr<MaterialData>> material_data
  void add_face_material_idx(MeshSource* mesh,std::vector<int32_t>&& material_idx) override;
  
  
  ////////////// Node Operations //////////////
  void add_tranform3x4(Node* node, std::vector<float>&& transform) override;
  void add_mesh_to_node(Node* node, const MeshSource* MeshSource) override;
  
  ////////////// Import Operations //////////////
  
  std::shared_ptr<Node>       create_node(const Node* parent, const std::string& name) override;
  std::shared_ptr<MeshSource> create_mesh(const std::string& name) override;
  
  void add_materials(const std::vector< std::shared_ptr<MaterialData> >& material_data) override;
  const std::shared_ptr<MaterialData> get_material( int idx ) override;
  int  apply_material(MeshSource* mesh, const MaterialData* material) override;
  void add_mdl_path(const std::string& mdl_path) override;
  void add_texture_path(std::unordered_set<std::string>&&) override;
  
  void serialize_to_file(const std::string& file_path, bool flattern);
  
protected:
  std::vector<std::shared_ptr<MeshSource>> _mesh_sources;
  std::vector<std::shared_ptr<Node>> _nodes;
  std::vector<std::shared_ptr<MaterialData>> _materials;
  std::unordered_set<std::string> _textures;
};



#endif /* MeshImporter_hpp */

//
//  MeshImport.h
//  sketchup_converter
//
//  Created by jahsia on 11/8/18.
//  Copyright © 2018 trisetra. All rights reserved.
//

#ifndef MeshImport_h
#define MeshImport_h
#include <vector>
#include <unordered_set>
#include <Eigen/Core>

namespace trisetra {
  
struct MaterialData{
  std::string name;
  std::string base_color_map;
  float base_color[4];
  float opacity;
};

class MeshSource{
public:
  MeshSource() = default;
  ~MeshSource() = default;
  std::string name;
  
  std::vector<float>  pos;
  std::vector<float>  normal;
  std::vector<float>  uv;
  std::vector<uint32_t> index;
  std::vector<const MaterialData* > materials;
  std::vector<int32_t> face_material_idx;
};

class Node{
public:
  virtual ~Node(){};
  
  Eigen::Matrix4f matrix;
  const Node* parent = nullptr;
  const MeshSource* mesh = nullptr;
};


class MeshImport{
public:
  MeshImport() = default;
  ~MeshImport() = default;
  
  ////////////// Mesh Operations //////////////
  virtual void add_positions(MeshSource* mesh, std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) = 0;
  virtual void add_normals(MeshSource* mesh,std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) = 0;
  virtual void add_tangent(MeshSource* mesh,std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) = 0;
  virtual void add_binormal(MeshSource* mesh,std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) = 0;
  //  idx of channel. 0,1,2,3 = RGBA, 0,1,2,-1 = RGB. specify -1 to ignore channel
  virtual void add_color(MeshSource* mesh,int r_idx, int g_idx, int b_idx, int a_idx, std::vector<float>&& src , std::vector<uint32_t>&& idx_buffer) = 0;
  virtual void add_uv(MeshSource* mesh, uint32_t idx, std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer) = 0;
  
  
  // indicate number of vertcies per face
  virtual void add_face_descriptor(MeshSource* mesh,std::vector<uint8_t>&& vert_per_face) = 0;
  // material_idx is used to access add_materials std::vector<std::shared_ptr<MaterialData>> material_data
  virtual void add_face_material_idx(MeshSource* mesh,std::vector<int32_t>&& material_idx) = 0;
  
  
  ////////////// Node Operations //////////////
  virtual void add_tranform3x4(Node* node, std::vector<float>&& transform) = 0;
  virtual void add_mesh_to_node(Node* node, const MeshSource* MeshSource) = 0;
  
  ////////////// Import Operations //////////////
  
  virtual std::shared_ptr<Node>       create_node(const Node* parent, const std::string& name) = 0;
  virtual std::shared_ptr<MeshSource> create_mesh(const std::string& name) = 0;
  
  virtual void add_materials(const std::vector< std::shared_ptr<MaterialData> >& material_data) = 0;
  virtual const std::shared_ptr<MaterialData> get_material( int idx ) = 0;
  virtual int  apply_material(MeshSource* mesh, const MaterialData* material) = 0;
  virtual void add_mdl_path(const std::string& mdl_path) = 0;
  virtual void add_texture_path(std::unordered_set<std::string>&&) = 0;
};

}
#endif /* MeshImport_h */

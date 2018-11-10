//
//  MeshImporter.cpp
//  sketchup_converter
//
//  Created by jahsia on 11/8/18.
//  Copyright © 2018 trisetra. All rights reserved.
//

#include "MeshImporter.hpp"
#include <fstream>
#include <iostream>

  ////////////// Mesh Operations //////////////
  void MeshImporter::add_positions(MeshSource* mesh, std::vector<float>&& src, std::vector<uint32_t>&& idx_buffer){
    mesh->pos = std::move(src);
    mesh->index = std::move(idx_buffer);
  }

  void MeshImporter::add_normals(MeshSource* mesh,std::vector<float>&& src, std::vector<uint32_t>&& ){
    mesh->normal = std::move(src);
  }

  void MeshImporter::add_tangent(MeshSource* mesh,std::vector<float>&& src, std::vector<uint32_t>&& ){
    
  }

  void MeshImporter::add_binormal(MeshSource* mesh,std::vector<float>&& src, std::vector<uint32_t>&& ){
    
  }

  //  idx of channel. 0,1,2,3 = RGBA, 0,1,2,-1 = RGB. specify -1 to ignore channel
  void MeshImporter::add_color(MeshSource* mesh,int r_idx, int g_idx, int b_idx, int a_idx, std::vector<float>&& src , std::vector<uint32_t>&& idx_buffer){
    
    
  }
  
  void MeshImporter::add_uv(MeshSource* mesh, uint32_t idx, std::vector<float>&& src, std::vector<uint32_t>&& ){
    mesh->uv = std::move(src);
  }
  
  
  // indicate number of vertcies per face
  void MeshImporter::add_face_descriptor(MeshSource* mesh,std::vector<uint8_t>&& vert_per_face){
    
  }
  // material_idx is used to access add_materials std::vector<std::shared_ptr<MaterialData>> material_data
  void MeshImporter::add_face_material_idx(MeshSource* mesh,std::vector<int32_t>&& material_idx){
    mesh->face_material_idx = std::move(material_idx);
  }
  
  
  ////////////// Node Operations //////////////
  void MeshImporter::add_tranform3x4(Node* node, std::vector<float>&& transform){
    node->matrix = Eigen::Matrix4f::Identity();
    std::vector<float> m = std::move(transform);
    node->matrix << m[0], m[1], m[2], 0.0f,
                    m[3], m[4], m[5], 0.0f,
                    m[6], m[7], m[8], 0.0f,
                    m[9], m[10],m[11], 1.0f;
  }

  void MeshImporter::add_mesh_to_node(Node* node, const MeshSource* MeshSource){
    node->mesh = MeshSource;
  }
  
  ////////////// Import Operations //////////////
  
  std::shared_ptr<Node>  MeshImporter::create_node(const Node* parent, const std::string& name){
    auto node = std::make_shared<Node>();
    node->parent = parent;
    _nodes.push_back(node);
    return _nodes.back();
  }
  std::shared_ptr<MeshSource> MeshImporter::create_mesh(const std::string& name){
    
    auto mesh = std::make_shared<MeshSource>();
    mesh->name = name;
    _mesh_sources.push_back(mesh);
    return _mesh_sources.back();
  }
  
  void MeshImporter::add_materials(const std::vector< std::shared_ptr<MaterialData> >& material_data){
    _materials.insert(_materials.begin(), material_data.begin(), material_data.end());
  }

  const std::shared_ptr<MaterialData> MeshImporter::get_material( int idx ){
    return _materials[idx];
  }

  int  MeshImporter::apply_material(MeshSource* mesh, const MaterialData* material){
    mesh->materials.push_back(material);
    return (int)(mesh->materials.size()-1);
  }

  void MeshImporter::add_mdl_path(const std::string& mdl_path){
    
  }

  void MeshImporter::add_texture_path(std::unordered_set<std::string>&& texts){
    _textures = std::move(texts);
  }

  void MeshImporter::serialize_to_file(const std::string& file_path, bool flattern){
    
    std::vector<Eigen::Vector3f> pos;
    std::vector<Eigen::Vector3f> normal;
    std::vector<float> uv;
    std::vector<uint32_t> indecies;
    
    for(auto node : _nodes){
      if(node->parent){
        node->matrix = node->parent->matrix * node->matrix;
      }
      
      if(node->mesh){
        if(flattern){
          const Eigen::Vector3f* pos_src = (const Eigen::Vector3f*)node->mesh->pos.data();
          const Eigen::Vector3f* norm_src = (const Eigen::Vector3f*)node->mesh->normal.data();
          const int* index = (const int*)node->mesh->index.data();
          std::vector< Eigen::Vector3f> pos_temp(node->mesh->pos.size()/3);
          std::vector< Eigen::Vector3f> norm_temp(node->mesh->normal.size()/3);
          
          Eigen::Vector4f temp;
          for(size_t i = 0; i < node->mesh->pos.size()/3; ++i){
            temp.head<3>() = pos_src[i];
            pos_temp[i] = (node->matrix*temp).head<3>();
            temp.head<3>() = norm_src[i];
            norm_temp[i] = (node->matrix*temp).head<3>();
          }
          
          pos.insert( pos.begin(), pos_temp.begin(), pos_temp.end() );
          normal.insert( normal.begin(),norm_temp.begin(), norm_temp.end() );
          uv.insert(uv.begin(), node->mesh->uv.begin(), node->mesh->uv.end() );
          
          int offset = indecies.size();
          std::vector<uint32_t> index_temp = node->mesh->index;
          for(size_t i = 0; i < node->mesh->index.size(); ++i){
            index_temp[i] += offset;
          }
          indecies.insert(indecies.begin(),index_temp.begin(), index_temp.end());
          
        }
      }
    }
    
    std::ofstream outfile;
    outfile.open (file_path, std::ios::out | std::ios::app | std::ios::binary);
    outfile << "TRIS";
    outfile << (uint32_t)pos.size()*3;
    for(size_t i = 0; i < pos.size(); ++i){
      outfile<<pos[i].x();
      outfile<<pos[i].y();
      outfile<<pos[i].z();
    }
    for(size_t i = 0; i < normal.size(); ++i){
      outfile<<normal[i].x();
      outfile<<normal[i].y();
      outfile<<normal[i].z();
    }
    for(size_t i = 0; i < uv.size(); ++i){
      outfile<<uv[i];
    }
    
    outfile<< (uint32_t)indecies.size();
    for(size_t i = 0; i < indecies.size(); ++i){
      outfile<<indecies[i];
    }
    outfile.close();
    
  }

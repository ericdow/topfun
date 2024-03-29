#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <numeric>

#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL.h>
// assimp includes
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"

#include "model/Mesh.h"

namespace TopFun {

// TODO move
GLint TextureFromFile(const char* path, const std::string& directory);

class Model {
 public:
  // Constructor, expects a filepath to a 3D model
  Model(const std::string& path, bool join_verts = false) {
    LoadModel(path, join_verts);
    // Default: draw meshes in order of appearance in .obj file
    draw_order_.resize(meshes_.size());
    std::iota(draw_order_.begin(), draw_order_.end(), 0);
    // Gather the total number of textures for all meshes
    num_textures_ = 0;
    for (auto const& mesh : meshes_) {
      num_textures_ += mesh.GetNumTextures();
    }
    // Initialize model matrices to point to NULL
    pmodel_matrices_ = std::vector<const glm::mat4*>(meshes_.size(), NULL);
    FormBoundingVolumes();
  }

  // Draws all the meshes in this model
  void Draw(const Shader* shader=NULL) {
    for(size_t i : draw_order_) {
      const Shader* current_shader;
      if (!shader) {
        current_shader = shaders_[i];
      }
      else {
        current_shader = shader;
      }
      current_shader->Use();
      glUniformMatrix4fv(glGetUniformLocation(current_shader->GetProgram(), 
            "model"), 1, GL_FALSE, glm::value_ptr(*(pmodel_matrices_[i])));
      meshes_[i].Draw(*current_shader);
    }
  }

  // Sets the order to draw the meshes in
  void SetDrawOrder(const std::vector<unsigned int>& draw_order) {
    draw_order_ = draw_order;
  }
  
  // Sets the shader pointer for each mesh
  void SetShaders(const std::vector<const Shader*>& shaders) {
    shaders_ = shaders;
  }
  
  // Sets the model oreintation matrix for a mesh
  void SetModelMatrix(const glm::mat4* pm, int mesh_idx) {
    pmodel_matrices_[mesh_idx] = pm;
  }

  inline GLuint GetNumTextures() const { return num_textures_; }
  
  inline GLuint GetNumMeshes() { return meshes_.size(); }
  
  inline const std::array<std::array<float,2>,3>& GetAABB() const {
    return AABB_;
  }

  inline const std::vector<Vertex>& GetVertices(int i) const {
    return meshes_[i].GetVertices();
  }
  
 private:
  std::vector<Mesh> meshes_;
  std::vector<unsigned int> draw_order_; // order to draw the meshes
  std::vector<const Shader*> shaders_; // shaders to use for each mesh
  std::vector<const glm::mat4*> pmodel_matrices_; // mesh orientations
  std::array<std::array<float,2>,3> AABB_; // min/max extent for x,y,z
  GLuint num_textures_; 

  // Loads a model with supported ASSIMP extensions from file
  void LoadModel(const std::string& path, bool join_verts) {
    // Read file via ASSIMP
    Assimp::Importer importer;
    auto pp_steps = aiProcess_Triangulate | aiProcess_FlipUVs | 
          aiProcess_CalcTangentSpace;
    if (join_verts)
      pp_steps = aiProcess_JoinIdenticalVertices;
    const aiScene* scene = importer.ReadFile(path, pp_steps);
    // Check for errors
    if (!scene || 
        scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || 
        !scene->mRootNode) {
      std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
      return;
    }
    // Retrieve the directory path of the filepath
    std::string directory = path.substr(0, path.find_last_of('/'));

    // Process ASSIMP's root node recursively
    ProcessNode(scene->mRootNode, scene, directory);
  }

  // Processes a node recursively
  void ProcessNode(aiNode* node, const aiScene* scene, 
      const std::string& directory) {
    // Process each mesh located at the current node
    for(GLuint i = 0; i < node->mNumMeshes; ++i) {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
      meshes_.push_back(ProcessMesh(mesh, scene, directory));			
    }
    // Recursively process each of the children nodes
    for(GLuint i = 0; i < node->mNumChildren; ++i) {
      ProcessNode(node->mChildren[i], scene, directory);
    }
  }

  Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, 
      const std::string& directory) {
    // Data to fill
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    // Walk through each of the mesh's vertices
    for (GLuint i = 0; i < mesh->mNumVertices; ++i) {
      Vertex vertex;
      glm::vec3 vector; 
      // Positions
      vector.x = mesh->mVertices[i].x;
      vector.y = mesh->mVertices[i].y;
      vector.z = mesh->mVertices[i].z;
      vertex.Position = vector;
      // Normals
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.Normal = vector;
      // Texture Coordinates
      if (mesh->mTextureCoords[0]) {
        glm::vec2 vec;
        vec.x = mesh->mTextureCoords[0][i].x; 
        vec.y = mesh->mTextureCoords[0][i].y;
        vertex.TexCoords = vec;
      }
      else
        vertex.TexCoords = glm::vec2(0.0f, 0.0f);
      if (mesh->HasTangentsAndBitangents()) {
        // tangent
        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        vertex.Tangent = vector;
        // bitangent
        vector.x = mesh->mBitangents[i].x;
        vector.y = mesh->mBitangents[i].y;
        vector.z = mesh->mBitangents[i].z;
        vertex.Bitangent = vector;
      }
      vertices.push_back(vertex);
    }
    // Walk through each mesh face and retrieve corresponding vertex indices
    for(GLuint i = 0; i < mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];
      // Retrieve all indices of the face and store them in the indices vector
      for(GLuint j = 0; j < face.mNumIndices; j++)
        indices.push_back(face.mIndices[j]);
    }
    // Process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // Each diffuse texture should be named as 'texture_diffuseN' where N is
    // a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // Diffuse: texture_diffuseN
    // Specular: texture_specularN
    // Normal: texture_normalN

    // 1. Diffuse maps
    std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, 
        aiTextureType_DIFFUSE, "texture_diffuse", directory);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. Specular maps
    std::vector<Texture> specularMaps = LoadMaterialTextures(material, 
        aiTextureType_SPECULAR, "texture_specular", directory);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. Normal maps
    std::vector<Texture> normalMaps = LoadMaterialTextures(material, 
        aiTextureType_HEIGHT, "texture_normal", directory);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. Height maps
    std::vector<Texture> heightMaps = LoadMaterialTextures(material, 
        aiTextureType_AMBIENT, "texture_height", directory);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    
    // Return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
  }

  // Checks all material textures of a given type and loads the textures
  // The required info is returned as a Texture struct
  std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
      const std::string& typeName, const std::string& directory) {
    std::vector<Texture> textures;
    for (GLuint i = 0; i < mat->GetTextureCount(type); ++i) {
      aiString str;
      mat->GetTexture(type, i, &str);
      Texture texture;
      texture.id = TextureFromFile(str.C_Str(), directory);
      texture.type = typeName;
      texture.path = str;
      textures.push_back(texture);
    }
    return textures;
  }

  // Forms bounding volumes of a model from its constituent meshes
  void FormBoundingVolumes() {
    AABB_ = meshes_[0].FormAABB();
    for (size_t i = 1; i < meshes_.size(); ++i) {
      std::array<std::array<float,2>,3> AABB = meshes_[i].FormAABB();
      AABB_[0][0] = std::min(AABB_[0][0], AABB[0][0]);
      AABB_[0][1] = std::max(AABB_[0][1], AABB[0][1]);
      AABB_[1][0] = std::min(AABB_[1][0], AABB[1][0]);
      AABB_[1][1] = std::max(AABB_[1][1], AABB[1][1]);
      AABB_[2][0] = std::min(AABB_[2][0], AABB[2][0]);
      AABB_[2][1] = std::max(AABB_[2][1], AABB[2][1]);
    }
  }
};

inline GLint TextureFromFile(const char* path, const std::string& directory) {
  // Generate texture ID and load texture data 
  std::string filename = std::string(path);
  filename = directory + '/' + filename;
  GLuint textureID;
  glGenTextures(1, &textureID);
  int width,height;
  unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0,
      SOIL_LOAD_RGB);
  // Assign texture to ID
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, 
      GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);	

  // Parameters
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
      GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  SOIL_free_image_data(image);
  return textureID;
}

} // End namespace TopFun

#endif

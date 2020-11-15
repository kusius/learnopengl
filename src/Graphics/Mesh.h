#ifndef MESH_H
#define MESH_H

// A 3D mesh storage and rednering class

#include <string>
#include <vector>

#include "thirdparty/glm/glm.hpp"
#include "thirdparty/glad/glad.h"

#include "Shader.h"

using namespace std;

struct Vertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
};

struct Texture
{
  unsigned int id;
  string type;
  string path;
};

struct BoundingBox
{
  glm::vec3 mMin;
  glm::vec3 mMax;
};

class Mesh
{

public:
  /*Mesh Data*/
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vector<Texture> textures;
  BoundingBox boundingBox;
  /*Functions*/
  Mesh(vector<Vertex> vertices, vector<unsigned int> indices,
       vector<Texture> textures, BoundingBox boundingBox);
  void Draw(Shader *shader, Shader *highlightShader = nullptr);
  /*Other data*/
  GLboolean selected;

private:
  /*Render data*/
  unsigned int VAO, VBO, EBO;
  /*Functions*/
  void setupMesh();
};

#endif

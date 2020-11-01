#ifndef MESH_H
#define MESH_H


#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include <vector>

using namespace glm;

struct Vertex;
struct Texture;

class Mesh {
private:
    //  render data
    unsigned int VAO, VBO, EBO;
    void calcMassCenter ();
    void setupMesh ();
public:
    vec3 massCenter;
    vec4 AABB; // left, right, up, down
    // mesh data
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int getVAO ();
    Mesh (vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
    void Draw (Shader *shader, int amount);
};

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
    vec3 Tangent;
    vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    string type;
    string path;  // we store the path of the texture to compare with other textures
};

#endif

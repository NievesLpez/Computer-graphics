#pragma once
#include <GL/glew.h>
#include <vector>

//#include <glm/glm.hpp>

#include "glfunctions.h"
#include "tiny_obj_loader.h"

class ObjectPBR
{
public:
    ObjectPBR() = default;

    // Carga OBJ + crea VAO + carga texturas 
    bool Load(const char* objPath, GLuint shaderPBR,
        const char* baseColorPath,
        const char* normalPath,
        const char* roughnessPath,
        const char* aoPath);

    // Dibuja el objeto 
    void Draw(const glm::mat4& model,
        const glm::mat4& view,
        const glm::mat4& projection) const;


    GLuint GetVAO() const { return vao; }
    int GetNumTriangles() const { return numTriangles; }

private:
    bool LoadFromOBJ(const char* objPath, GLuint shaderPBR);
    GLuint LoadTexture2D(const char* path, bool flipY);

private:

    GLuint vao = 0;
    int numTriangles = 0;

    GLuint texBase = 0;
    GLuint texNormal = 0;
    GLuint texRough = 0;
    GLuint texAO = 0;

    GLuint shader = 0;
    std::vector<tinyobj::shape_t> shapes;
};

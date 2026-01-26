#include "ObjectPBR.h"
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"

bool ObjectPBR::Load(const char* objPath, GLuint shaderPBR,
    const char* baseColorPath,
    const char* normalPath,
    const char* roughnessPath,
    const char* aoPath)
{
    shader = shaderPBR;

    // 1) OBJ + VAO
    if (!LoadFromOBJ(objPath, shaderPBR))
        return false;

    // 2) Texturas 
    texBase = LoadTexture2D(baseColorPath, true);
    texNormal = LoadTexture2D(normalPath, true);
    texRough = LoadTexture2D(roughnessPath, true);


    if (aoPath && aoPath[0] != '\0') texAO = LoadTexture2D(aoPath, true);
    else texAO = 0;

    return true;
}

bool ObjectPBR::LoadFromOBJ(const char* objPath, GLuint shaderPBR)
{
    shapes.clear();

    bool ret = tinyobj::LoadObj(shapes, objPath);
    if (!ret || shapes.empty())
    {
        std::cout << "OBJ File: " << objPath << " cannot be found or is not valid\n";
        return false;
    }

    vao = gl_createAndBindVAO();

    // Posiciones
    gl_createAndBindAttribute(&(shapes[0].mesh.positions[0]),
        (int)(shapes[0].mesh.positions.size() * sizeof(float)),
        shaderPBR, "a_vertex", 3);

    // UV 
    if (!shapes[0].mesh.texcoords.empty())
    {
        gl_createAndBindAttribute(&(shapes[0].mesh.texcoords[0]),
            (int)(shapes[0].mesh.texcoords.size() * sizeof(GLfloat)),
            shaderPBR, "a_uv", 2);
    }

    if (!shapes[0].mesh.normals.empty())
    {
        gl_createAndBindAttribute(&(shapes[0].mesh.normals[0]),
            (int)(shapes[0].mesh.normals.size() * sizeof(float)),
            shaderPBR, "a_normal", 3);
    }

    // Indices
    gl_createIndexBuffer(&(shapes[0].mesh.indices[0]),
        (int)(shapes[0].mesh.indices.size() * sizeof(unsigned int)));

    gl_unbindVAO();

    numTriangles = (int)shapes[0].mesh.indices.size() / 3;

    std::cout << "OBJ File: " << objPath << " sucessfully loaded\n";
    return true;
}

GLuint ObjectPBR::LoadTexture2D(const char* path, bool flipY)
{
    if (!path || path[0] == '\0')
        return 0;

    int w, h, n;
    stbi_set_flip_vertically_on_load(flipY ? 1 : 0);
    unsigned char* pixels = stbi_load(path, &w, &h, &n, 0);

    if (!pixels)
    {
        std::cout << "ERROR: texture map couldn't be loaded: " << path
            << " -> " << stbi_failure_reason() << "\n";
        return 0;
    }

    GLenum fmt = GL_RGB;
    if (n == 1) fmt = GL_RED;
    else if (n == 3) fmt = GL_RGB;
    else if (n == 4) fmt = GL_RGBA;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, pixels);

    stbi_image_free(pixels);
    return tex;
}

void ObjectPBR::Draw(const glm::mat4& model,
    const glm::mat4& view,
    const glm::mat4& projection) const
{
    if (vao == 0 || numTriangles == 0) return;

    glUseProgram(shader);

    // Matrices
    GLint locM = glGetUniformLocation(shader, "u_model");
    GLint locV = glGetUniformLocation(shader, "u_view");
    GLint locP = glGetUniformLocation(shader, "u_projection");

    if (locM >= 0) glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(model));
    if (locV >= 0) glUniformMatrix4fv(locV, 1, GL_FALSE, glm::value_ptr(view));
    if (locP >= 0) glUniformMatrix4fv(locP, 1, GL_FALSE, glm::value_ptr(projection));

    // Buscar uniforms de texturas
    GLint uTex = glGetUniformLocation(shader, "u_texture");
    GLint uTexN = glGetUniformLocation(shader, "u_texture_normal");
    GLint uTexR = glGetUniformLocation(shader, "u_texture_rough");
    GLint uTexAO = glGetUniformLocation(shader, "u_texture_ao");

    // base = asigna texturas a texture units
    if (uTex >= 0) glUniform1i(uTex, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texBase);

    // normal
    if (uTexN >= 0) glUniform1i(uTexN, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texNormal);

    // roughness
    if (uTexR >= 0) glUniform1i(uTexR, 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texRough);

    // ao (si no hay, bind 0)
    if (uTexAO >= 0) glUniform1i(uTexAO, 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texAO);

    gl_bindVAO(vao);
    glDrawElements(GL_TRIANGLES, 3 * numTriangles, GL_UNSIGNED_INT, 0);
}


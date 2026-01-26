#pragma once
#include <GL/glew.h>

namespace tex
{
    // Carga una textura
    GLuint LoadTexture2D(const char* path,
        bool flipVertical = true,
        GLint minFilter = GL_LINEAR,
        GLint magFilter = GL_LINEAR);
}

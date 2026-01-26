#include "TextureUtils.h"
#include <iostream>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace tex
{
    static GLenum ChannelsToFormat(int nChannels)
    {
        if (nChannels == 1) return GL_RED;
        if (nChannels == 3) return GL_RGB;
        if (nChannels == 4) return GL_RGBA;
        return GL_RGB;
    }

    GLuint LoadTexture2D(const char* path, bool flipVertical, GLint minFilter, GLint magFilter)
    {
        int w = 0, h = 0, n = 0;
        stbi_set_flip_vertically_on_load(flipVertical);

        unsigned char* pixels = stbi_load(path, &w, &h, &n, 0); 
        if (!pixels)
        {
            std::cout << "ERROR: no se pudo cargar textura " << path
                << " -> " << stbi_failure_reason() << "\n";
            return 0;
        }

        GLenum fmt = ChannelsToFormat(n);

        GLuint id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, pixels);

        stbi_image_free(pixels);
        return id;
    }
}



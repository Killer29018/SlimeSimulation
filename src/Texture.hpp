#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glad/glad.h>

class Texture2D
{
public:
    unsigned int ID;
    unsigned int width, height;
    unsigned int internalFormat;
    unsigned int imageFormat;

    unsigned int wrapS;
    unsigned int wrapT;
    unsigned int filterMin;
    unsigned int filterMax;
public:
    Texture2D()
        : width(0), height(0), internalFormat(GL_RGB), imageFormat(GL_RGB), wrapS(GL_REPEAT), wrapT(GL_REPEAT),
        filterMin(GL_LINEAR), filterMax(GL_LINEAR)
    {
        glGenTextures(1, &ID);
    }

    void generate(unsigned int width, unsigned int height, unsigned char* data)
    {
        this->width = width;
        this->height = height;

        // Create Texture
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, data);

        // Set Texture Wrap and Filters Modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMin);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMax);

        // unbind Texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void bind() const
    {
        glBindTexture(GL_TEXTURE_2D, ID);
    }
};

#endif
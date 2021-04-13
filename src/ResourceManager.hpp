#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <iostream>
#include <map>

#include "Shader.hpp"
#include "Texture.hpp"

class ResourceManager
{
private:
    static std::map<const char*, Shader> shaders;
    static std::map<const char*, Texture2D> textures;
public:
    static Shader loadShader(const char* name, const char* vertexPath, const char* fragmentPath, const char* geometryPath = NULL);
    static Texture2D loadTexture(const char* name, const char* filePath, bool alpha = false);

    static Shader& getShader(const char* name);
    static Texture2D& getTexture(const char* name);

    static void clear();
private:
    ResourceManager() {}
};

#endif
#ifndef TEXT_RENDERER_HPP
#define TEXT_RENDERER_HPP

#include <GLAD/glad.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <GLM/glm.hpp>

#include <iostream>
#include <map>

#include "Shader.hpp"
#include "Texture.hpp"
#include "ResourceManager.hpp"

class TextRenderer
{
private:
    static std::map<std::string, TTF_Font*> fonts;
    static Shader shader;
 
    static unsigned int textureID;
    static unsigned int VAO, VBO;
    
    static SDL_Surface* surface;
public:
    static void init(unsigned int width, unsigned int height, const char* vertexPath, const char* fragmentPath, const char* shaderName);
    static void clear();
    static void load(std::string fontName, std::string fontPath, unsigned int fontSize);
    static void renderText(std::string text, std::string fontName, glm::vec2 pos, float scale, glm::vec3 colour);
private:
    static void generateBufferData();
};

#endif
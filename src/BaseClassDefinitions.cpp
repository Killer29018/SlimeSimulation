#include "ResourceManager.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TextRenderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

// Resource Manager
std::map<const char*, Shader> ResourceManager::shaders;
std::map<const char*, Texture2D> ResourceManager::textures;
Shader ResourceManager::loadShader(const char* name, const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
    Shader shader;
    shader.compileFromPath(vertexPath, fragmentPath, geometryPath);
    shaders[name] = shader;
    return shaders[name];
}

Texture2D ResourceManager::loadTexture(const char* name, const char* filePath, bool alpha)
{
    Texture2D texture;
    if (alpha)
    {
        texture.imageFormat = GL_RGBA;
        texture.internalFormat = GL_RGBA;
    }

    int width, height, channels;
    unsigned char* data = stbi_load(filePath, &width, &height, &channels, 0);
    texture.generate(width, height, data);
    stbi_image_free(data);

    textures[name] = texture;
    return textures[name];
}

Shader& ResourceManager::getShader(const char* name) 
{
    return shaders[name]; 
}

Texture2D& ResourceManager::getTexture(const char* name) 
{
    return textures[name];
}

void ResourceManager::clear()
{
    for (auto& shader : shaders)
        glDeleteProgram(shader.second.ID);
    for (auto& texture : textures)
        glDeleteTextures(1, &texture.second.ID);
}

// Text Renderer
std::map<std::string, TTF_Font*> TextRenderer::fonts;
Shader TextRenderer::shader;
unsigned int TextRenderer::textureID;
unsigned int TextRenderer::VAO;
unsigned int TextRenderer::VBO;
SDL_Surface* TextRenderer::surface;

void TextRenderer::init(unsigned int width, unsigned int height, const char* vertexPath, const char* fragmentPath, const char* shaderName)
{
    TTF_Init();
    shader = ResourceManager::loadShader(shaderName, vertexPath, fragmentPath);
    shader.setMatrix4("projection", glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f), true);
    shader.setInt("text", 0);

    generateBufferData();
}

void TextRenderer::clear()
{
    for (auto& font : fonts)
    {
        TTF_CloseFont(font.second);
    }
    fonts.clear();
}

void TextRenderer::load(std::string fontName, std::string fontPath, unsigned int fontSize) 
{
    fonts[fontName] = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!fonts[fontName])
    {
        std::cerr << "Unable to Open Font: " << fontName << std::endl;
    }
}

void TextRenderer::renderText(std::string text, std::string fontName, glm::vec2 pos, float scale, glm::vec3 colour)
{
    shader.use();
    
    SDL_Color c;
    c.r = colour.r * 255;
    c.g = colour.g * 255;
    c.b = colour.b * 255;
    c.a = 255;

    glBindTexture(GL_TEXTURE_2D, textureID);

    surface = TTF_RenderText_Blended(fonts[fontName], text.c_str(), c);

    int mode = GL_RGB;
    if (surface->format->BytesPerPixel == 4)
        mode = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);

    float x = pos.x;
    float y = pos.y;
    float w = surface->w * scale;
    float h = surface->h * scale;

    float verticies[] = {
        // Position     TexCoords
        x    , y + h,   0.0f, 1.0f,
        x + w, y    ,   1.0f, 0.0f,
        x    , y    ,   0.0f, 0.0f,

        x    , y + h,   0.0f, 1.0f,
        x + w, y + h,   1.0f, 1.0f,
        x + w, y    ,   1.0f, 0.0f
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::generateBufferData()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}
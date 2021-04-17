#ifndef SHADER_HPP
#define SHADER_HPP

#include <GLAD/glad.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;
public:
    Shader() {}

    void compileFromPath(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;

        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            std::stringstream vertexStream, fragmentStream, geometryStream;
            
            file.open(vertexPath);
            vertexStream << file.rdbuf();
            file.close();
            vertexCode = vertexStream.str();

            file.open(fragmentPath);
            fragmentStream << file.rdbuf();
            file.close();
            fragmentCode = fragmentStream.str();

            if (geometryPath)
            {
                file.open(geometryPath);
                geometryStream << file.rdbuf();
                file.close();
                geometryCode = geometryStream.str();
            }
        }
        catch (std::ifstream::failure* e)
        {
            std::cerr << "ERROR:SHADER::FILE_READ_ERROR" << std::endl;
        }

        const char* vertexSource = vertexCode.c_str();
        const char* fragmentSource = fragmentCode.c_str();
        const char* geometrySource = geometryCode.c_str();

        unsigned int sVertex, sFragment, gShader;

        // Vertex Shader
        sVertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(sVertex, 1, &vertexSource, NULL);
        glCompileShader(sVertex);
        checkCompileErrors(sVertex, "VERTEX");

        // Fragment Shader
        sFragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(sFragment, 1, &fragmentSource, NULL);
        glCompileShader(sFragment);
        checkCompileErrors(sFragment, "FRAGMENT");

        // Geometry Shader
        if (geometryPath)
        {
            gShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(gShader, 1, &geometrySource, NULL);
            glCompileShader(gShader);
            checkCompileErrors(gShader, "GEOMETRY");
        }

        // Create the Shader
        ID = glCreateProgram();
        glAttachShader(ID, sVertex);
        glAttachShader(ID, sFragment);
        if (geometryPath) glAttachShader(ID, gShader);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // Delete the Shaders
        glDeleteShader(sVertex);
        glDeleteShader(sFragment);
        if (geometryPath) glDeleteShader(gShader);
    }

    void compileFromSource(const char* vertexSource, const char* fragmentSource, const char* geometrySource = nullptr)
    {
        unsigned int sVertex, sFragment, gShader;

        // Vertex Shader
        sVertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(sVertex, 1, &vertexSource, NULL);
        glCompileShader(sVertex);
        checkCompileErrors(sVertex, "VERTEX");

        // Fragment Shader
        sFragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(sFragment, 1, &fragmentSource, NULL);
        glCompileShader(sFragment);
        checkCompileErrors(sFragment, "FRAGMENT");

        // Geometry Shader
        if (geometrySource)
        {
            gShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(gShader, 1, &geometrySource, NULL);
            glCompileShader(gShader);
            checkCompileErrors(gShader, "GEOMETRY");
        }

        // Create the Shader
        ID = glCreateProgram();
        glAttachShader(ID, sVertex);
        glAttachShader(ID, sFragment);
        if (geometrySource) glAttachShader(ID, gShader);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // Delete the Shaders
        glDeleteShader(sVertex);
        glDeleteShader(sFragment);
        if (geometrySource) glDeleteShader(gShader);
    }

    Shader& use() { glUseProgram(ID); return *this; }

    void setFloat(const char* name, float value, bool useShader = false) 
        { if(useShader) use(); glUniform1f(glGetUniformLocation(ID, name), value); }
    
    void setInt(const char* name, int value, bool useShader = false) 
        { if(useShader) use(); glUniform1i(glGetUniformLocation(ID, name), value); }
    
    void setVector2f(const char* name, float x, float y, bool useShader = false)
        { if(useShader) use(); glUniform2f(glGetUniformLocation(ID, name), x, y); }
    
    void setVector2f(const char* name, const glm::vec2& value, bool useShader = false)
        { if(useShader) use(); glUniform2f(glGetUniformLocation(ID, name), value.x, value.y); }
    
    void setVector3f(const char* name, float x, float y, float z, bool useShader = false)
        { if(useShader) use(); glUniform3f(glGetUniformLocation(ID, name), x, y, z); }
    
    void setVector3f(const char* name, const glm::vec3& value, bool useShader = false)
        { if(useShader) use(); glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z); }
    
    void setVector4f(const char* name, float x, float y, float z, float w, bool useShader = false)
        { if(useShader) use(); glUniform4f(glGetUniformLocation(ID, name), x, y, z, w); }
    
    void setVector4f(const char* name, const glm::vec4& value, bool useShader = false)
        { if(useShader) use(); glUniform4f(glGetUniformLocation(ID, name), value.x, value.y, value.z, value.w); }
    
    void setMatrix4(const char* name, glm::mat4 matrix, bool useShader = false)
        { if(useShader) use(); glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, false, glm::value_ptr(matrix)); }

protected:
    void checkCompileErrors(unsigned int object, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(object, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(object, 1024, NULL, infoLog);
                std::cerr << "| ERROR::SHADER: Compile-time Error: Type: " << type << "\n" << infoLog << std::endl << std::endl;
            }
        }
        else
        {
            glGetProgramiv(object, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(object, 1024, NULL, infoLog);
                std::cerr << "| ERROR::SHADER: Link-time Error: Type: " << type << "\n" << infoLog << std::endl << std::endl;
            }
        }
    }
};

class ComputeShader : public Shader
{
public:
    ComputeShader() {}

    void compileFromPath(const char* computePath)
    {
        std::string computeCode;

        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            std::stringstream computeStream;
            
            file.open(computePath);
            computeStream << file.rdbuf();
            file.close();
            computeCode = computeStream.str();
        }
        catch (std::ifstream::failure* e)
        {
            std::cerr << "ERROR:SHADER::FILE_READ_ERROR" << std::endl;
        }

        const char* computeSource = computeCode.c_str();

        unsigned int sCompute;

        // Compute Shader
        sCompute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(sCompute, 1, &computeSource, NULL);
        glCompileShader(sCompute);
        checkCompileErrors(sCompute, "COMPUTE");

        // Create the Shader
        ID = glCreateProgram();
        glAttachShader(ID, sCompute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // Delete the Shaders
        glDeleteShader(sCompute);
    }

    void compileFromSource(const char* computeSource)
    {
        unsigned int sCompute;

        // Compute Shader
        sCompute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(sCompute, 1, &computeSource, NULL);
        glCompileShader(sCompute);
        checkCompileErrors(sCompute, "COMPUTE");

        // Create the Shader
        ID = glCreateProgram();
        glAttachShader(ID, sCompute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // Delete the Shaders
        glDeleteShader(sCompute);
    }

    void addStorageBuffer(const char* name, int binding, unsigned int ssbo, unsigned int bufferIndex = 1)
    {
        int index = glGetProgramResourceIndex(ID, GL_SHADER_STORAGE_BUFFER, "bufferData");
        glShaderStorageBlockBinding(ID, index, binding);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferIndex, ssbo);
    }
};

#endif
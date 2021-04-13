#include <GLAD/glad.h>
#include <SDL2/SDL.h>

#include <GLM/glm.hpp>

#include <iostream>
#include <string>

#include "Shader.hpp"

#include <vector>
#include <ctime>

/*
SOURCES:
https://uwe-repository.worktribe.com/output/980579
https://www.sagejenson.com/physarum
*/

const unsigned int SCREEN_WIDTH  = 1920;
const unsigned int SCREEN_HEIGHT = 1080;

const unsigned int TEXTURE_WIDTH  = 1920; // 960 1920 2560
const unsigned int TEXTURE_HEIGHT = 1080; // 540 1080 1440

const float DECAY_AMOUNT = 0.3f;
const float DIFFUSE_SPEED = 0.1f;
const float MOVEMENT_DISTANCE = 10.0f;

const float SENSOR_DISTANCE = 4.0f;
const float SENSOR_ANGLE = 45.0f;
const float ROTATION = 45.0f;

const int SPAWN_RADIUS = 500;

const int AGENT_COUNT = 2500000;

struct agent
{
    glm::vec3 pos;
    float angle;
};

#define PI 3.14159
float degToRad(float deg)
{
    return deg * (PI / 180.0);
}

float radToDeg(float rad)
{
    return rad * (180.0 / PI);
}

void generateTexture(unsigned int& id, unsigned int width, unsigned int height, unsigned int binding, GLenum access);

agent generateInwardCircle(int radius = 100);

agent generateOutwardCircle(int radius = 100);

agent generateRandom();

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow("SlimeMouldSimulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
    SDL_GL_SetSwapInterval(0);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int texture;
    generateTexture(texture, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_READ_WRITE);

    unsigned int output;
    generateTexture(output, TEXTURE_WIDTH, TEXTURE_HEIGHT, 1, GL_READ_WRITE);

    Shader basic;
    basic.compileFromPath("res/Shaders/vertexShader.glsl", "res/Shaders/fragmentShader.glsl");
    basic.setInt("tex", 0);

    ComputeShader basicShader;
    basicShader.compileFromPath("res/Shaders/basicComputeShader.glsl");

    ComputeShader agentShader;
    agentShader.compileFromPath("res/Shaders/agentComputeShader.glsl");

    ComputeShader diffuseDecayShader;
    diffuseDecayShader.compileFromPath("res/Shaders/diffuseDecayCompute.glsl");

    ComputeShader colourShader;
    colourShader.compileFromPath("res/Shaders/colourComputeShader.glsl");

    srand(time(0));
    std::vector<agent> agents;
    for (int i = 0; i < AGENT_COUNT; i++)
    {
        agent a = generateOutwardCircle(SPAWN_RADIUS);
        agents.push_back(a);
    }   

    unsigned int ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, agents.size() * sizeof(agent), &agents[0], GL_DYNAMIC_DRAW);

    float vertexData[] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int FBO;
    glGenFramebuffers(1, &FBO);

    float deltaTime = 0.0f;
    float lastTime = 0.0f;

    // basicShader.use();
    // glDispatchCompute(80, 80, 1);
    // glMemoryBarrier(GL_ALL_BARRIER_BITS);

    bool running = true;
    bool paused = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
            {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE)
                    running = false;
                if (key == SDLK_SPACE)
                    paused = false;
                break;
            }
            }
        }

        float currentTime = SDL_GetTicks() / 1000.0f;
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!paused)
        {
            agentShader.use();
            agentShader.addStorageBuffer("bufferData", 1, ssbo);
            agentShader.setInt("agentCount", AGENT_COUNT);
            agentShader.setFloat("movementDistance", MOVEMENT_DISTANCE);
            agentShader.setFloat("deltaTime", deltaTime);
            agentShader.setFloat("sensorDistance", SENSOR_DISTANCE);
            agentShader.setFloat("sensorAngle", SENSOR_ANGLE);
            agentShader.setFloat("rotationAngle", ROTATION);
            glDispatchCompute(AGENT_COUNT, 1, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            diffuseDecayShader.use();
            diffuseDecayShader.setFloat("decayAmount", DECAY_AMOUNT);
            diffuseDecayShader.setFloat("diffuseSpeed", DIFFUSE_SPEED);
            diffuseDecayShader.setFloat("deltaTime", deltaTime);
            glDispatchCompute(TEXTURE_WIDTH / 8, TEXTURE_HEIGHT / 8, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            colourShader.use();
            colourShader.setVector3f("targetColour", glm::vec3(0.03f, 0.98f, 0.98f));
            glDispatchCompute(TEXTURE_WIDTH / 8, TEXTURE_HEIGHT / 8, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);


            glCopyImageSubData(output, GL_TEXTURE_2D, 0, 0, 0, 0, texture, GL_TEXTURE_2D, 0, 0, 0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 1);

            basic.use();
            glBindVertexArray(VAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);

            glDrawArrays(GL_TRIANGLES, 0 ,6);
            glBindVertexArray(0);
        }

        SDL_GL_SwapWindow(window);

        // SDL_Delay(2000);
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

void generateTexture(unsigned int& id, unsigned int width, unsigned int height, unsigned int binding, GLenum access)
{
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(binding, id, 0, GL_FALSE, 0, access, GL_RGBA32F);
}

agent generateInwardCircle(int maxRadius)
{
    int radius = rand() % maxRadius;
    float angle = (rand() % (360 * 30)) / 30.0f;

    agent a;
    a.pos = glm::vec3(0.0f);
    a.pos.x = (TEXTURE_WIDTH  / 2) + (radius * cos(degToRad(angle)));
    a.pos.y = (TEXTURE_HEIGHT / 2) + (radius * sin(degToRad(angle)));
    a.pos.z = 0.0;

    a.angle = angle + 180.0;

    return a;
}

agent generateOutwardCircle(int maxRadius)
{
    int radius = rand() % maxRadius;
    float angle = (rand() % (360 * 30)) / 30.0f;

    agent a;
    a.pos = glm::vec3(0.0f);
    a.pos.x = (TEXTURE_WIDTH  / 2) + (radius * cos(degToRad(angle)));
    a.pos.y = (TEXTURE_HEIGHT / 2) + (radius * sin(degToRad(angle)));
    a.pos.z = 0.0;

    a.angle = angle;

    return a;
}

agent generateRandom()
{
    agent a;
    a.pos = glm::vec3(rand() % TEXTURE_WIDTH, rand() % TEXTURE_HEIGHT, 0.0);
    a.angle = rand() % 360;
    return a;
}

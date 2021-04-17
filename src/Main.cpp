#include <GLAD/glad.h>
#include <SDL2/SDL.h>

#include <GLM/glm.hpp>

#include <iostream>
#include <string>

#include "Shader.hpp"

#include <vector>
#include <ctime>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <array>

/*
SOURCES:
https://uwe-repository.worktribe.com/output/980579
https://www.sagejenson.com/physarum
*/

const unsigned int SCREEN_WIDTH  = 1280;
const unsigned int SCREEN_HEIGHT = 720;

const unsigned int TEXTURE_WIDTH  = 1280; // 960 1280 1920 2560
const unsigned int TEXTURE_HEIGHT = 720; // 540 720 1080 1440

const float DEFAULT_DECAY_AMOUNT = 0.3f;
const float DEFAULT_DIFFUSE_SPEED = 0.3f;
const float DEFAULT_MOVEMENT_DISTANCE = 10.0f;

const float DEFAULT_SENSOR_DISTANCE = 4.0f;
const float DEFAULT_SENSOR_ANGLE = 45.0f;
const float DEFAULT_ROTATION = 45.0f;

const int DEFAULT_SPAWN_RADIUS = TEXTURE_HEIGHT / 2;

const int DEFAULT_AGENT_COUNT = 2500000;

float DECAY_AMOUNT, DIFFUSE_SPEED, MOVEMENT_DISTANCE;
float SENSOR_DISTANCE, SENSOR_ANGLE, ROTATION;
int SPAWN_RADIUS;
int AGENT_COUNT;

struct agent
{
    glm::vec3 pos;
    float angle;
};

enum class generationType
{
    IN_CIRCLE,
    OUT_CIRCLE,
    RANDOM
};

generationType generation = generationType::IN_CIRCLE;

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

void resetValues();

void reset(unsigned int ssbo, unsigned int fbo, unsigned int texture);

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

    SDL_Window* window = SDL_CreateWindow("SlimeMouldSimulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 330");

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

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);

    unsigned int ssbo;
    glGenBuffers(1, &ssbo);

    resetValues();
    reset(ssbo, fbo, texture);

    float deltaTime = 0.0f;
    float lastTime = 0.0f;

    // basicShader.use();
    // glDispatchCompute(80, 80, 1);
    // glMemoryBarrier(GL_ALL_BARRIER_BITS);

    const char* generationTypeLabels[] = { "Inward Circle", "Outward Circle", "Random" };
    int generationIndex = 0;

    ImVec4 slimeColour = ImVec4(175.0f / 255.0f, 217.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);

    bool running = true;
    bool paused = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        float currentTime = SDL_GetTicks() / 1000.0f;
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ImGui::ShowDemoWindow();

        ImGui::Begin("Settings", NULL);
        ImGui::Dummy(ImVec2(1.0f, 1.0f));
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Information");

        ImGui::Text("FPS: %.2f", 1 / deltaTime);
        ImGui::Text("Delta time: %.5f", deltaTime);

        ImGui::Text("Generation Type:");
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
        if (ImGui::BeginCombo("", generationTypeLabels[generationIndex], 0))
        {
            for (int i = 0; i < IM_ARRAYSIZE(generationTypeLabels); i++)
            {
                bool isSelected = (i == generationIndex);
                if (ImGui::Selectable(generationTypeLabels[i], isSelected))
                {
                    generationIndex = i;
                    
                    switch (i)
                    {
                    case (int)generationType::IN_CIRCLE:
                        generation = generationType::IN_CIRCLE;
                        break;
                    case (int)generationType::OUT_CIRCLE:
                        generation = generationType::OUT_CIRCLE;
                        break;
                    case (int)generationType::RANDOM:
                        generation = generationType::RANDOM;
                        break;
                    }
                }
                
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SliderFloat("Decay Amount", &DECAY_AMOUNT, 0.0f, 1.0f, "%.3f", 0);
        ImGui::SliderFloat("Diffuse Speed", &DIFFUSE_SPEED, 0.0f, 1.0f, "%.3f", 0);
        ImGui::SliderFloat("Movement Distance", &MOVEMENT_DISTANCE, 2.0f, 15.0f, "%.3f", 0);

        ImGui::SliderFloat("Sensor Distance", &SENSOR_DISTANCE, 1.0f, 8.0f, "%.3f", 0);
        ImGui::SliderFloat("Sensor Angle", &SENSOR_ANGLE, 10.0f, 90.0f, "%.3f", 0);
        ImGui::SliderFloat("Rotation", &ROTATION, 5.0f, 45.0f, "%.3f", 0);

        ImGui::SliderInt("Spawn Radius", &SPAWN_RADIUS, 0, SCREEN_HEIGHT, "%d", 0);
        ImGui::SliderInt("Agent Count", &AGENT_COUNT, 1000000, 5000000, "%d", 0);

        ImGui::Text("Color widget:");
        ImGui::ColorEdit4("Slime Colour", (float*)&slimeColour, 0);

        ImGui::Text("Paused: %s", paused ? "True" : "False");
        if (ImGui::Button("Toggle"))
        {
            paused = !paused;
        }

        if (ImGui::Button("Reset"))
        {
            reset(ssbo, fbo, texture);
        }

        if (ImGui::Button("Reset Values"))
        {
            resetValues();
        }

        ImGui::End();

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
            colourShader.setVector4f("targetColour", glm::vec4(slimeColour.x, slimeColour.y, slimeColour.z, slimeColour.w));
            glDispatchCompute(TEXTURE_WIDTH / 8, TEXTURE_HEIGHT / 8, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            glCopyImageSubData(output, GL_TEXTURE_2D, 0, 0, 0, 0, texture, GL_TEXTURE_2D, 0, 0, 0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 1);
        }

        basic.use();
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glDrawArrays(GL_TRIANGLES, 0 ,6);
        glBindVertexArray(0);

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(context);
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

void resetValues()
{
    DECAY_AMOUNT = DEFAULT_DECAY_AMOUNT;
    DIFFUSE_SPEED = DEFAULT_DIFFUSE_SPEED;
    MOVEMENT_DISTANCE = DEFAULT_MOVEMENT_DISTANCE;
    SENSOR_DISTANCE = DEFAULT_SENSOR_DISTANCE;
    SENSOR_ANGLE = DEFAULT_SENSOR_ANGLE;
    ROTATION = DEFAULT_ROTATION;
    SPAWN_RADIUS = DEFAULT_SPAWN_RADIUS;
    AGENT_COUNT = DEFAULT_AGENT_COUNT;
}

void reset(unsigned int ssbo, unsigned int fbo, unsigned int texture)
{
    std::vector<agent> agents;
    for (int i = 0; i < AGENT_COUNT; i++)
    {
        agent a;
        switch (generation)
        {
        case generationType::IN_CIRCLE: a = generateInwardCircle(SPAWN_RADIUS); break;
        case generationType::OUT_CIRCLE: a = generateOutwardCircle(SPAWN_RADIUS); break;
        case generationType::RANDOM: a = generateRandom(); break;
        }
        
        agents.push_back(a);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, agents.size() * sizeof(agent), &agents[0], GL_DYNAMIC_DRAW);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

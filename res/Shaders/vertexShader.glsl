#version 330 core
layout (location = 0) in vec4 vertexData;
out vec2 texCoords;

void main()
{
    texCoords = vertexData.zw;
    gl_Position = vec4(vertexData.xy, 0.0, 1.0);
}
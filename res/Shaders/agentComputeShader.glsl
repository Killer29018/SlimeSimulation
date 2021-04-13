#version 430

struct agent
{
    vec3 pos;
    float angle;
};

layout (local_size_x = 1, local_size_y = 1) in;

layout (binding = 0, rgba32f) uniform image2D texture;

layout (std430, binding = 1) buffer bufferData
{
    agent agents[];
};

uniform int agentCount;
uniform float movementDistance;
uniform float deltaTime;
uniform float sensorDistance;
uniform float sensorAngle;
uniform float rotationAngle;

float random(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float strength(vec4 colour)
{
    return colour.r + colour.g + colour.b;
}

void main()
{
    ivec2 px = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(texture);

    agent a = agents[px.x];

    vec2 pos = a.pos.xy;
    vec2 newPos;

    // Movement Stage
    newPos.x = pos.x + (movementDistance * cos(radians(a.angle)) * deltaTime);
    newPos.y = pos.y + (movementDistance * sin(radians(a.angle)) * deltaTime);

    float rnd = random(newPos);

    if (newPos.x >= size.x || newPos.x <= 0 || newPos.y >= size.y || newPos.y <= 0)
    {
        newPos = clamp(newPos, vec2(0.0), vec2(size - 1));
        float newAngle = 180 + (rnd * 30.0 - 15.0);
        agents[px.x].angle = newAngle;
    }

    agents[px.x].pos = vec3(newPos, 0.0);

    imageStore(texture, ivec2(pos), vec4(1.0));

    // Sensory Stage
    ivec2 positionI;
    vec2 position;
    position.x = pos.x + (sensorDistance * cos(radians(a.angle)));
    position.y = pos.y + (sensorDistance * sin(radians(a.angle)));
    positionI = ivec2(position.x, position.y);
    float front = strength(imageLoad(texture, positionI));

    position.x = pos.x + (sensorDistance * cos(radians(a.angle - sensorAngle)));
    position.y = pos.y + (sensorDistance * sin(radians(a.angle - sensorAngle)));
    positionI = ivec2(position.x, position.y);
    float frontLeft = strength(imageLoad(texture, positionI));

    position.x = pos.x + (sensorDistance * cos(radians(a.angle + sensorAngle)));
    position.y = pos.y + (sensorDistance * sin(radians(a.angle + sensorAngle)));
    positionI = ivec2(position.x, position.y);
    float frontRight = strength(imageLoad(texture, positionI));

    if (front < frontLeft && front < frontRight) // Rotate Randomly
    {
        float r = random(newPos);
        if (r < 0.5) // Rotate Left
            agents[px.x].angle -= rotationAngle * rnd;
        else // Rotate Right
            agents[px.x].angle += rotationAngle * rnd;
    }
    else if (frontLeft > frontRight) // Rotate Left
    {
        agents[px.x].angle -= rotationAngle * rnd;
    }
    else if (frontRight > frontLeft) // Rotate Right
    {
        agents[px.x].angle += rotationAngle * rnd;
    }
    else // Stay Forwards
    {

    }
}


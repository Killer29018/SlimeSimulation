#version 430 

layout (local_size_x = 10, local_size_y = 10) in;

layout (binding = 0, rgba32f) uniform image2D outTexture;

float random(float value)
{
    return fract(sin(value) * 43758.5453123);
}

float random(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main()
{
    ivec2 px = ivec2(gl_GlobalInvocationID.xy);

    float rand = random(px);
    imageStore(outTexture, px, vec4(rand, rand, rand, 1.0));
}
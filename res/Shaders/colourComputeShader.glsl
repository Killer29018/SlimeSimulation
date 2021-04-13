#version 430

layout (local_size_x = 8, local_size_y = 8) in;

layout (binding = 1, rgba32f) uniform image2D texture;

uniform vec3 targetColour;

void main()
{
    ivec2 px = ivec2(gl_GlobalInvocationID.xy);

    vec4 newColour = vec4(targetColour, 1.0);
    vec4 colour = imageLoad(texture, px);

    colour *= newColour;

    imageStore(texture, px, colour);
}


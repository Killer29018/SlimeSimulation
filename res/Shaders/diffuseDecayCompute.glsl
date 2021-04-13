#version 430

layout (local_size_x = 8, local_size_y = 8) in;

layout (binding = 0, rgba32f) uniform image2D inputTexture;
layout (binding = 1, rgba32f) uniform image2D outputTexture;

uniform float decayAmount;
uniform float diffuseSpeed;
uniform float deltaTime;

void main()
{
    ivec2 px = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(inputTexture);

    vec4 original = imageLoad(inputTexture, px);
    vec4 colour = original;

    int kernal = 1;

    // Diffuse
    if (px.x >= 1 && px.y >= 1 && px.x < size.x && px.y < size.y)
    {
        float count = 1.0;
        for (int i = -kernal; i <= kernal; i++)
        {
            for (int j = -kernal; j <= kernal; j++)
            {
                if (i == 0 && j == 0)
                    continue;
                
                colour += imageLoad(inputTexture, ivec2(px.x + i, px.y + j));
                count += 1.0;
            }
        }

        colour /= count;
    }


    colour = mix(original, colour, diffuseSpeed);

    vec4 final = max(vec4(0.0), colour - decayAmount * deltaTime);

    imageStore(outputTexture, px, vec4(final.rgb, 1.0));
}
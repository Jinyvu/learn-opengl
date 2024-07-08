
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

vec4 apply3x3Kernel(sampler2D sourceTexture, vec2 coord, float offset, float[9] kernel)
{
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset),  // top-left
        vec2(0.0f, offset),     // top-center
        vec2(offset, offset),   // top-right
        vec2(-offset, 0.0f),    // center-left
        vec2(0.0f, 0.0f),       // center-center
        vec2(offset, 0.0f),     // center-right
        vec2(-offset, -offset), // bottom-left
        vec2(0.0f, -offset),    // bottom-center
        vec2(offset, -offset)   // bottom-right
    );

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(sourceTexture, coord + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++)
    {
        col += sampleTex[i] * kernel[i];
    }
    return vec4(col, 1.0);
}

/** 反色 */
vec4 inversion(vec3 color)
{
    return vec4(vec3(1.0 - color), 1.0);
}

/** 灰度 */
vec4 grayscale(vec3 color)
{
    // adjust, the human eye tends to be more sensitive to green colors and the least to blue
    //  float ave = (color.r + color.g + color.b) / 3.0;
    float ave = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    return vec4(ave, ave, ave, 1.0);
}

/** 锐化 */
vec4 sharpen(sampler2D sourceTexture, vec2 coord)
{
    const float offset = 1.0 / 300.0;
    float kernel[9] = float[](
        -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1);
    return apply3x3Kernel(sourceTexture, coord, offset, kernel);
}

/** 模糊 */
vec4 blur(sampler2D sourceTexture, vec2 coord)
{
    const float offset = 1.0 / 300.0;
    float kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16);
    return apply3x3Kernel(sourceTexture, coord, offset, kernel);
}

/** 边缘检测 */
vec4 edgeDetection(sampler2D sourceTexture, vec2 coord) {
    const float offset = 1.0 / 300.0;
    float kernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1);
    return apply3x3Kernel(sourceTexture, coord, offset, kernel);
}

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    // FragColor = vec4(col, 1.0);
    // FragColor = inversion(col);
    // FragColor = grayscale(col);
    // FragColor = sharpen(screenTexture, TexCoords.st);
    // FragColor = blur(screenTexture, TexCoords.st);
    FragColor = edgeDetection(screenTexture, TexCoords.st);
}
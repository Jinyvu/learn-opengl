#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{    
    vec4 texColor1 = texture(texture1, TexCoords);
    vec4 texColor2 = texture(texture2, TexCoords);

    // Screen blending formula
    vec4 blendedColor = 1.0 - (1.0 - texColor1) * (1.0 - texColor2);
    FragColor = blendedColor;
}
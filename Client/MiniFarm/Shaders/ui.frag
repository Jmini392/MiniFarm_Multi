#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uTexture;
uniform vec4 uColor;

uniform float uMax;

void main()
{
    if (TexCoords.x > uMax)
        discard;

    vec4 texColor = texture(uTexture, TexCoords);
    FragColor = texColor * uColor;
}

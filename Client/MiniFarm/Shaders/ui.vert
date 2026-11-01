#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 TexCoords;

uniform vec2 uPos;
uniform vec2 uSize;

void main()
{
    vec2 pos = aPos * uSize + uPos;  
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);

    TexCoords = aUV;
}

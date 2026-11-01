#version 330 core
layout (location = 0) in vec4 aVertex;

out vec2 vUV;

uniform mat4 uProj;

void main()
{
    vec2 pos = aVertex.xy;
    vUV = aVertex.zw;

    gl_Position = uProj * vec4(pos, 0.0, 1.0);
}

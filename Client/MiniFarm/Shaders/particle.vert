#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aVelocity;
layout (location = 2) in float aLife;

out VS_OUT {
    float life;
} vs_out;

uniform mat4 uView;

void main()
{
    gl_Position = uView * vec4(aPos, 1.0);
    vs_out.life = aLife;
}
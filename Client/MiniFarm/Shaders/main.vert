#version 330 core

layout(location = 0) in vec3 aPos;    
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aTextCoord;

uniform mat4 uModel;
uniform mat4 uView;  
uniform mat4 uProj;  

uniform int uDebugMode;
uniform vec3 uDebugColor;

out vec3 vColor;
out vec2 vTextCoord;
out vec3 vNormal;
out vec3 vPos;

void main()
{
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
    
    vTextCoord = aTextCoord;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vPos = vec3(uModel * vec4(aPos, 1.0));

    if (uDebugMode == 1)
        vColor = uDebugColor;
    else
        vColor = aColor;
}
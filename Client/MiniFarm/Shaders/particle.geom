#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 24) out;

in VS_OUT {
    float life;
} gs_in[];

out vec3 FragPos;
out vec3 Normal;
flat out float Life;

uniform mat4 uProj;
uniform mat4 uView;

void BuildSphere(vec3 center, float radius)
{
    Life = gs_in[0].life;
    
    Normal = vec3(0, 0, 1);
    gl_Position = uProj * vec4(center + vec3(-radius, -radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, -radius, radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, -radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, -radius, radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(-radius, radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, radius, radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, radius, radius), 1.0)).xyz;
    EmitVertex();
    EndPrimitive();
    
    // µÞ¸é
    Normal = vec3(0, 0, -1);
    gl_Position = uProj * vec4(center + vec3(radius, -radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, -radius, -radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(-radius, -radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, -radius, -radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, radius, -radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(-radius, radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, radius, -radius), 1.0)).xyz;
    EmitVertex();
    EndPrimitive();
    
    Normal = vec3(0, 1, 0);
    gl_Position = uProj * vec4(center + vec3(-radius, radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, radius, radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, radius, radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(-radius, radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, radius, -radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, radius, -radius), 1.0)).xyz;
    EmitVertex();
    EndPrimitive();
    
    Normal = vec3(0, -1, 0);
    gl_Position = uProj * vec4(center + vec3(-radius, -radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, -radius, -radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, -radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, -radius, -radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(-radius, -radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, -radius, radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, -radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, -radius, radius), 1.0)).xyz;
    EmitVertex();
    EndPrimitive();
    
    Normal = vec3(1, 0, 0);
    gl_Position = uProj * vec4(center + vec3(radius, -radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, -radius, radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, -radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, -radius, -radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, radius, radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(radius, radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(radius, radius, -radius), 1.0)).xyz;
    EmitVertex();
    EndPrimitive();
    
    Normal = vec3(-1, 0, 0);
    gl_Position = uProj * vec4(center + vec3(-radius, -radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, -radius, -radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(-radius, -radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, -radius, radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(-radius, radius, -radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, radius, -radius), 1.0)).xyz;
    EmitVertex();
    gl_Position = uProj * vec4(center + vec3(-radius, radius, radius), 1.0);
    FragPos = (uView * vec4(center + vec3(-radius, radius, radius), 1.0)).xyz;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    if (gs_in[0].life <= 0.0)
        return;
    
    vec3 center = gl_in[0].gl_Position.xyz;
    float radius = 0.05;
    
    BuildSphere(center, radius);
}
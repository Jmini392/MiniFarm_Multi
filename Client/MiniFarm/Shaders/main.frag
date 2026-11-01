#version 330 core

in vec3 vColor;
in vec2 vTextCoord;
in vec3 vNormal;
in vec3 vPos;

uniform int uDebugMode;
out vec4 FragColor;

uniform sampler2D atexture;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uMoonPos;
uniform vec3 uMoonColor;
uniform vec3 uViewPos;

vec3 CalcLight(vec3 lightPos, vec3 lightColor, vec3 norm, vec3 viewDir)
{
    vec3 lightDir = normalize(lightPos - vPos);
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Ambient
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * lightColor;
    
    return diffuse + specular + ambient;
}

void main()
{
    if (uDebugMode == 1)
    {
        FragColor = vec4(vColor, 1.0);
    }
    else
    {
        vec4 baseColor = texture(atexture, vTextCoord);
        vec3 norm = normalize(vNormal);
        vec3 viewDir = normalize(uViewPos - vPos);
        
        vec3 sunLight = CalcLight(uLightPos, uLightColor, norm, viewDir);
        vec3 moonLight = CalcLight(uMoonPos, uMoonColor, norm, viewDir);
        
        vec3 result = (sunLight + moonLight) * baseColor.rgb;
        FragColor = vec4(result, baseColor.a);
    }
}
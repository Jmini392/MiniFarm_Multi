#version 330 core
in vec3 FragPos;
in vec3 Normal;
flat in float Life;

out vec4 FragColor;

uniform vec3 uLightPos;
uniform vec3 uViewPos;

void main()
{
    vec3 waterColor = vec3(0.2, 0.5, 1.0);
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    vec3 ambient = 0.3 * waterColor;
    vec3 diffuse = 0.5 * diff * waterColor;
    vec3 specular = 0.8 * spec * vec3(1.0);
    
    vec3 result = ambient + diffuse + specular;
    
    float alpha = Life * 0.8;
    
    FragColor = vec4(result, alpha);
}
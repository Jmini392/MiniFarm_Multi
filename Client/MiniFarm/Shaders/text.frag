#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uFont;
uniform vec3 uTextColor;

void main()
{
    vec4 tex = texture(uFont, vUV);

    float alpha = tex.r;

    if (alpha < 0.1)
        discard;

    FragColor = vec4(uTextColor, 1.0) * alpha;
}

#version 330 core

in vec2 V_Texcoord;
in vec4 V_Color;
in vec4 V_Normal;
uniform sampler2D U_Texture;
out vec4 FragColor;

vec4 GammaCorrection(vec4 color) {
    float gamma = 2.2f;
    vec3 result = pow(color.rgb, vec3(1.0f / gamma));
    return vec4(result, 1.0f);
}

void main()
{
    vec3 color = texture(U_Texture, V_Texcoord).rgb;
    FragColor = GammaCorrection(V_Normal);
}

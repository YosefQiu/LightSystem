#version 330 core

in vec2 V_Texcoord;
in vec4 V_Color;
in vec4 V_Normal;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gTex;
out vec4 FragColor;

vec4 GammaCorrection(vec4 color) {
    float gamma = 2.2f;
    vec3 result = pow(color.rgb, vec3(1.0f / gamma));
    return vec4(result, 1.0f);
}

vec2 CalcTexCoord() {
    return gl_FragCoord.xy / vec2(1280.0f, 720.0f);
}

void main()
{
    vec2 Tex = CalcTexCoord();
    vec3 worldPos = texture(gPosition, Tex.xy).rgb;
    vec3 N = texture(gNormal, Tex).rgb;
    FragColor = vec4(N, 1.0f);
}

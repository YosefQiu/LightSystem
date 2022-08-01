#version 330 core

in vec2 V_Texcoord;
in vec4 V_Color;
in vec4 V_Normal;
in vec4 V_WorldPos;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gTex;
layout(location = 3) out vec4 gDepth;


vec4 GammaCorrection(vec4 color) {
    float gamma = 2.2f;
    vec3 result = pow(color.rgb, vec3(1.0f / gamma));
    return vec4(result, 1.0f);
}

const float NEAR = 0.1f;
const float FAR = 100.0f;
float LinearizeDepth(float depth) {
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
}

void main()
{
    gPosition.xyz = V_WorldPos.xyz;
    gPosition.w = LinearizeDepth(gl_FragCoord.z);
    gNormal = normalize(V_Normal);
    gTex = vec4(V_Texcoord.xy, 1.0f, 1.0f);
    gDepth = vec4(V_WorldPos.z);
}

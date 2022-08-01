#version 330 core
#ifdef GL_ES
precision mediump float;
#endif
in vec4 V_Color;
in vec4 V_Normal;
in vec4 V_WorldPos;

uniform vec4 U_lightColor;

out vec4 FragColor;

// -----------------------------------------------------
// utility functions
// -----------------------------------------------------
const float gamma = 2.2f;
vec3 PowVec3(vec3 v, float p) {
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}
vec3 ToLinear(vec3 v) {
    return PowVec3(v, gamma);
}
vec4 ToLinear(vec4 v) {
    return vec4(ToLinear(v.xyz), 1.0f);
}

void main()
{
    FragColor = ToLinear(U_lightColor);
}

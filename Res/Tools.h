#ifndef TOOLS_H
#define TOOLS_H
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
vec3 IntegrateEdgeVec(vec3 v1, vec3 v2) {
    // Using built-in acos() function will result flaws
    // Using fitting result for calculating acos()
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985f + (0.4965155f + 0.0145206f * y) * y;
    float b = 3.4175940f + (4.1616724f + y) * y;
    float v = a / b;
    float theta_sintheta = (x > 0.0f) ? v : 0.5f * inversesqrt(max(1.0f - x * x, 1e-7)) - v;
    return cross(v1, v2) * theta_sintheta;
}

float IntegrateEdge(vec3 v1, vec3 v2) {
    return IntegrateEdgeVec(v1, v2).z;
}

float Fpo(float d, float l) {
    return l / (d * (d * d + l * l)) + atan(l / d) / (d * d);
}

float Fwt(float d, float l) {
    return l * l / (d * (d * d + l * l));
}

#endif // !TOOLS_H




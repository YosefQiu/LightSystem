#version 330 core
#ifdef GL_ES
precision mediump float;
#endif

#define NUM_LIGHTS 4
#define NUM_POINTS 4
#define MAX_SPHERE_LIGHTS 100




// Area Light
struct RALight {
    float U_Intensity;
    vec4 U_LightColor;
    vec4 U_Points[NUM_POINTS];
};

struct TALight {
    float U_Radius;
    float U_Intensity;
    vec4 U_LightColor;
    vec4 U_Points[2];
};

struct SALight {
    float U_Intensity;
    vec4 U_LightColor;
    vec4 U_Points[NUM_POINTS];
};

struct Material {
    vec4 U_Ambient;
    vec4 U_Diffuse;
    vec4 U_Specular;
    float U_Shininess;
    float U_Roughness;
};

in vec4 V_Color;
in vec4 V_Normal;
in vec4 V_WorldPos;
in vec4 V_Texcoord;
in vec4 V_LightSpaceFragPos;

uniform bool U_bTexture;
uniform sampler2D U_Texture;
uniform sampler2D U_LTC1Map; // for inverse M
uniform sampler2D U_LTC2Map; // GGX norm, fresnel, 0(unused), sphere
uniform sampler2D gPosition; // xyz position w depth
uniform sampler2D gNormal;

uniform vec4 U_CameraPos;

// area light
uniform RALight U_RALight;
uniform TALight U_TALight;
uniform SALight U_SALight;
uniform Material U_Material;

uniform mat4 U_ChooseLight;  // for calc which light
uniform mat4 U_ChooseLightShadow; // whether to turn on the shadow

const float Pi = 3.14159265359f;
const float LUT_SIZE = 64.0f; //ltc_texture size
const float LUT_SCALE = (LUT_SIZE - 1.0f) / LUT_SIZE;
const float LUT_BIAS = 0.5f / LUT_SIZE;

#define MEDIUMP_FLT_MAX 65504.0
#define saturate(x) min(x, MEDIUMP_FLT_MAX)

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 HdrColor;

#include "Tools.h"
#include "Shadow.h"
#include "TheoryLight.h"


float I_diffuse_line(vec3 p1, vec3 p2) {
    vec3 wt = normalize(p2 - p1);
    if (p1.z <= 0.0 && p2.z <= 0.0) return 0.0;
    if (p1.z < 0.0) p1 = (+p1*p2.z - p2*p1.z) / (+p2.z - p1.z);
    if (p2.z < 0.0) p2 = (-p1*p2.z + p2*p1.z) / (-p2.z + p1.z);

    float l1 = dot(p1, wt);
    float l2 = dot(p2, wt);

    vec3 po = p1 - l1*wt;

    float d = length(po);

    float I = (Fpo(d, l2) - Fpo(d, l1)) * po.z +
              (Fwt(d, l2) - Fwt(d, l1)) * wt.z;
    return I / Pi;
}

float I_ltc_line(vec3 p1, vec3 p2, mat3 Minv) {
    // transform to diffuse configuration
    vec3 p1o = Minv * p1;
    vec3 p2o = Minv * p2;
    float I_diffuse = I_diffuse_line(p1o, p2o);
    
    // width factor
    vec3 ortho = normalize(cross(p1, p2));
    float w = 1.0f / length(inverse(transpose(Minv)) * ortho);
    
    return w * I_diffuse;
}

// disk LTC utility function
vec3 SolveCubic(vec4 coefficient) {
    // Normalize
    coefficient.xyz /= coefficient.w;
    coefficient.yz /= 3.0f;
    
    float A = coefficient.w;
    float B = coefficient.z;
    float C = coefficient.y;
    float D = coefficient.x;
    
    // compute the hessian and discriminatnt
    vec3 Delta = vec3(
                      -coefficient.z * coefficient.z + coefficient.y,
                      -coefficient.y * coefficient.z + coefficient.x,
                      dot(vec2(coefficient.z, -coefficient.y), coefficient.xy)
                      );
    float Discriminant = dot(vec2(4.0f * Delta.x, -Delta.y), Delta.zy);
    vec3 RootsA, RootsD;
    vec2 xlc, xsc;
    {
        float A_a =  1.0f;
        float C_a = Delta.x;
        float D_a = -2.0f * B * Delta.x + Delta.y;
        
        float Theta = atan(sqrt(Discriminant), -D_a) / 3.0f;
        
        float x_1a = 2.0f * sqrt(-C_a) * cos(Theta);
        float x_3a = 2.0f * sqrt(-C_a) * cos(Theta + (2.0f / 3.0f) * Pi);
        float xl;
        if ((x_1a + x_3a) > 2.0*B)
            xl = x_1a;
        else
            xl = x_3a;

        xlc = vec2(xl - B, A);
    }
    {
        float A_d = D;
        float C_d = Delta.z;
        float D_d = -D*Delta.y + 2.0*C*Delta.z;

        // Take the cubic root of a normalized complex number
        float Theta = atan(D*sqrt(Discriminant), -D_d)/3.0;

        float x_1d = 2.0*sqrt(-C_d)*cos(Theta);
        float x_3d = 2.0*sqrt(-C_d)*cos(Theta + (2.0/3.0)*Pi);

        float xs;
        if (x_1d + x_3d < 2.0*C)
            xs = x_1d;
        else
            xs = x_3d;

        xsc = vec2(-D, xs + C);
    }
    float E =  xlc.y*xsc.y;
    float F = -xlc.x*xsc.y - xlc.y*xsc.x;
    float G =  xlc.x*xsc.x;

    vec2 xmc = vec2(C*F - B*G, -B*F + C*E);

    vec3 Root = vec3(xsc.x/xsc.y, xmc.x/xmc.y, xlc.x/xlc.y);

    if (Root.x < Root.y && Root.x < Root.z)
        Root.xyz = Root.yxz;
    else if (Root.z < Root.x && Root.z < Root.y)
        Root.xyz = Root.xzy;

    return Root;
}

vec3 LTC_Evaluate_Polygon(vec3 N, vec3 V, vec3 P, mat3 MinV, vec4 points[4]) {
    
    /*
    **@brief: LTC distributi
    **@params:
    *** P is fragpos in world space --> V_WorldPos.xyz
    **@ret:
    **@birth:created by YosefQiu
    */
    
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);
    
    //rotate area light in (T1, T2, N) basis
    MinV = MinV * transpose(mat3(T1, T2, N));
    
    // polygon(allocate 5 vertices for clipping)
    vec3 L[5];
    // transform polygon from LTC back to origin
    L[0] = MinV * (points[0].xyz - P);
    L[1] = MinV * (points[1].xyz - P);
    L[2] = MinV * (points[2].xyz - P);
    L[3] = MinV * (points[3].xyz - P);
    
    // integrate
    float sum = 0.0f;
    
    // use tabulated horizon-clipped sphere
    // check if the shading point is behind the light
    vec3 dir = points[0].xyz - P; // LTC space
    vec3 lightNormal = cross(points[1].xyz - points[0].xyz, points[3].xyz - points[0].xyz);
    bool behind = (dot(dir, lightNormal) < 0.0);

    // cos weighted space
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);
    
    vec3 vsum = vec3(0.0f);
    
    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);
    
    // form factor of the polygon in direction vsum
    float len = length(vsum);
    // TODO: ???
    float z = vsum.z/len;
    
    // TODO: ???
    if (behind)
        z = -z;
    
    vec2 uv = vec2(z*0.5 + 0.5, len); // range [0, 1]
    uv = uv * LUT_SCALE + LUT_BIAS;
    
    // TODO: ???
    float scale = texture(U_LTC2Map, uv).w;
    
    sum = len*scale;
    
    // Out irradiance ???
    vec3 Lo_i = vec3(sum, sum, sum);

    return Lo_i;
}

vec3 LTC_Evaluate_Line(vec3 N, vec3 V, vec3 P, mat3 MinV, vec4 points[2], float radius) {
    vec3 T1, T2;
    
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);
    
    mat3 B = transpose(mat3(T1, T2, N));
    vec3 p1 = B * (points[0].xyz - P);
    vec3 p2 = B * (points[1].xyz - P);
    
    float Iline = radius * I_ltc_line(p1, p2, MinV);
    return vec3(min(1.0f, Iline));
}

vec3 LTC_Evaluate_Disk(vec3 N, vec3 V, vec3 P, mat3 MinV, vec4 points[4]) {
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    mat3 R = transpose(mat3(T1, T2, N));

    // 3 of the 4 vertices around disk
    vec3 L_[3];
    L_[0] = R * (points[0].xyz - P);
    L_[1] = R * (points[1].xyz - P);
    L_[2] = R * (points[2].xyz - P);

    // init ellipse
    vec3 C  = 0.5 * (L_[0] + L_[2]); // center
    vec3 V1 = 0.5 * (L_[1] - L_[2]); // axis 1
    vec3 V2 = 0.5 * (L_[1] - L_[0]); // axis 2

    // back to cosine distribution, but V1 and V2 no longer ortho.
    C  = MinV * C;
    V1 = MinV * V1;
    V2 = MinV * V2;

    // compute eigenvectors of ellipse
    float a, b;
    float d11 = dot(V1, V1); // q11
    float d22 = dot(V2, V2); // q22
    float d12 = dot(V1, V2); // q12
    if (abs(d12) / sqrt(d11*d22) > 0.0001f)
    {
        float tr = d11 + d22;
        float det = -d12*d12 + d11*d22;

        // use sqrt matrix to solve for eigenvalues
        det = sqrt(det);
        float u = 0.5*sqrt(tr - 2.0*det);
        float v = 0.5*sqrt(tr + 2.0*det);
        float e_max = (u + v) * (u + v); // e2
        float e_min = (u - v) * (u - v); // e1

        // two eigenvectors
        vec3 V1_, V2_;

        // q11 > q22
        if (d11 > d22)
        {
            V1_ = d12*V1 + (e_max - d11)*V2; // E2
            V2_ = d12*V1 + (e_min - d11)*V2; // E1
        }
        else
        {
            V1_ = d12*V2 + (e_max - d22)*V1;
            V2_ = d12*V2 + (e_min - d22)*V1;
        }

        a = 1.0 / e_max;
        b = 1.0 / e_min;
        V1 = normalize(V1_); // Vx
        V2 = normalize(V2_); // Vy
    }
    else
    {
        // Eigenvalues are diagnoals
        a = 1.0 / dot(V1, V1);
        b = 1.0 / dot(V2, V2);
        V1 *= sqrt(a);
        V2 *= sqrt(b);
    }

    vec3 V3 = cross(V1, V2);
    if (dot(C, V3) < 0.0)
        V3 *= -1.0;

    float L  = dot(V3, C);
    float x0 = dot(V1, C) / L;
    float y0 = dot(V2, C) / L;

    a *= L*L;
    b *= L*L;

    // parameters for solving cubic function
    float c0 = a*b;
    float c1 = a*b*(1.0 + x0*x0 + y0*y0) - a - b;
    float c2 = 1.0 - a*(1.0 + x0*x0) - b*(1.0 + y0*y0);
    float c3 = 1.0;

    // 3D eigen-decomposition: need to solve a cubic function
    vec3 roots = SolveCubic(vec4(c0, c1, c2, c3));

    float e1 = roots.x;
    float e2 = roots.y;
    float e3 = roots.z;

    // direction to front-facing ellipse center
    vec3 avgDir = vec3(a*x0/(a - e2), b*y0/(b - e2), 1.0); // third eigenvector: V-

    mat3 rotate = mat3(V1, V2, V3);

    // transform to V1, V2, V3 basis
    avgDir = rotate*avgDir;
    avgDir = normalize(avgDir);

    // extends of front-facing ellipse
    float L1 = sqrt(-e2/e3);
    float L2 = sqrt(-e2/e1);

    // projected solid angle E, like the length(F) in rectangle light
    float formFactor = L1*L2*inversesqrt((1.0 + L1*L1)*(1.0 + L2*L2));

    // use tabulated horizon-clipped sphere
    vec2 uv = vec2(avgDir.z*0.5 + 0.5, formFactor);
    uv = uv*LUT_SCALE + LUT_BIAS;
    float scale = texture(U_LTC2Map, uv).w;

    float spec = formFactor*scale;
    vec3 Lo_i = vec3(spec, spec, spec);

    return Lo_i;
}


vec2 CalcAreaUV(float NoV) {
    // use roughness and sqrt(1-cos_theta) to sample M_texture
    float roughness = max(0.1f, U_Material.U_Roughness);
    vec2 UV = vec2(roughness, sqrt(1.0f - NoV));
    UV = UV * LUT_SCALE + LUT_BIAS;
    return UV;
}

vec4 CalcRectAreaLight(vec3 mDiff, vec3 mSpec, vec3 N, vec3 V, vec3 WorldPos, vec4 points[4]) {
    float NoV = clamp(dot(N, V), 0.0f, 1.0f);
    vec2 UV = CalcAreaUV(NoV);
    
    vec4 tex1 = texture(U_LTC1Map, UV);
    vec4 tex2 = texture(U_LTC2Map, UV);
    
    mat3 MinV = mat3(
                 vec3(tex1.x, 0.0f, tex1.y),
                 vec3(0.0f, 1.0f, 0.0f),
                 vec3(tex1.z, 0.0f, tex1.w)
                 );
    
    // Evaluate LTC shading
    vec3 rect_diff = LTC_Evaluate_Polygon(N, V, WorldPos, mat3(1.0f), points);
    vec3 rect_spec = LTC_Evaluate_Polygon(N, V, WorldPos, MinV, points);
    
    // GGX BRDF shadowing and Fresnel
    // t2.x: shadowedF90 ??? (F90 normally it should be 1.0)
    // t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
    rect_spec *= mSpec * tex2.x + (1.0f - mSpec) * tex2.y;
    
    vec4 rect_color = vec4(U_RALight.U_Intensity * U_RALight.U_LightColor.xyz * (rect_spec + mDiff * rect_diff), 1.0f);
    
    return rect_color;
}

vec4 CalcCylinderAreaLight(vec3 mDiff, vec3 mSpec, vec3 N, vec3 V, vec3 WorldPos, vec4 points[2], float radius) {
    float NoV = clamp(dot(N, V), 0.0f, 1.0f);
    vec2 UV = CalcAreaUV(NoV);
    
    vec4 t1 = texture(U_LTC1Map, UV);
    vec4 t2 = texture(U_LTC2Map, UV);
    
    mat3 MinV = mat3(
                 vec3(t1.x, 0.0f, t1.y),
                 vec3(0.0f, 1.0f, 0.0f),
                 vec3(t1.z, 0.0f, t1.w)
                 );
    
    vec3 tube_diff = LTC_Evaluate_Line(N, V, WorldPos, mat3(1), points, radius);
    vec3 tube_spec = LTC_Evaluate_Line(N, V, WorldPos, MinV, points, radius);
    tube_spec *= mSpec * t2.x + (1.0f - mSpec) * t2.y;
    
    vec4 tube_color = vec4 (U_TALight.U_Intensity * U_TALight.U_LightColor.xyz * (tube_spec + mDiff * tube_diff), 1.0f);
    
    return tube_color;
}

vec4 CalcSphereAreaLight(vec3 mDiff, vec3 mSpec, vec3 N, vec3 V, vec3 WorldPos, vec4 points[4]) {
    float NoV = clamp(dot(N, V), 0.0f, 1.0f);
    vec2 UV = CalcAreaUV(NoV);
    
    vec4 t1 = texture(U_LTC1Map, UV);
    vec4 t2 = texture(U_LTC2Map, UV);
    
    mat3 MinV = mat3(
                 vec3(t1.x, 0.0f, t1.y),
                 vec3(0.0f, 1.0f, 0.0f),
                 vec3(t1.z, 0.0f, t1.w)
                 );
    
    // Evaluate LTC shading
    vec3 sphere_diff = LTC_Evaluate_Disk(N, V, WorldPos, mat3(1.0f), points);
    vec3 sphere_spec = LTC_Evaluate_Disk(N, V, WorldPos, MinV, points);
    
    // GGX BRDF shadowing and Fresnel
    // t2.x: shadowedF90 ??? (F90 normally it should be 1.0)
    // t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
    sphere_spec *= mSpec * t2.x + (1.0f - mSpec) * t2.y;
    
    vec4 sphere_color = vec4(U_SALight.U_Intensity * U_SALight.U_LightColor.xyz * (sphere_spec + mDiff * sphere_diff), 1.0f);
    
    return sphere_color;
}

vec2 CalcTexCoord() {
    return gl_FragCoord.xy / vec2(1280.0f, 720.0f);
}

void Volumetric() {
    vec3 yellow_light = vec3(1.0f, 198.0f / 255.0f, 107.0f / 255.0f);
    vec2 uv = CalcTexCoord();
    vec3 N = texture(gNormal, uv).xyz;
    vec3 pos = texture(gPosition, uv).xyz;
    float depth = texture(gPosition, uv).w;
    vec3 result = vec3(0.0f);


}

void main()
{
    vec4 color = vec4(1.0, 0.0, 0.0, 0.0);
    
    vec3 mDiff = ToLinear(U_Material.U_Diffuse.xyz);
    vec3 mSpec = ToLinear(U_Material.U_Specular.xyz);
    vec3 N = normalize(V_Normal.xyz);
    vec3 V = normalize(U_CameraPos.xyz - V_WorldPos.xyz);
    float NoV = clamp(dot(N, V), 0.0f, 1.0f);
  
    vec4 dir_color   = CalcDirLight(U_DLight, V_WorldPos.xyz, V_Normal.xyz, U_Material);
    vec4 point_color = CalcPointLight(U_PLight, V_WorldPos.xyz, V_Normal.xyz, U_Material);
    vec4 spot_color  = CalcSpotLight(U_FLight, V_WorldPos.xyz, V_Normal.xyz, U_Material);
    
    vec4 rect_color = CalcRectAreaLight(mDiff, mSpec, N, V, V_WorldPos.xyz, U_RALight.U_Points);
    
    vec4 cylinder_color = CalcCylinderAreaLight(mDiff, mSpec, N, V, V_WorldPos.xyz, U_TALight.U_Points, U_TALight.U_Radius);
    
    vec4 sphere_color = CalcSphereAreaLight(mDiff, mSpec, N, V, V_WorldPos.xyz, U_SALight.U_Points);
    
    vec4 area_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 theory_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Theory Light
    if (U_ChooseLight[0][0] == 1.0f) {
        if (U_ChooseLight[0][1] == 1.0f) {
            theory_color += dir_color;
        }
        if (U_ChooseLight[0][2] == 1.0f) {
            theory_color += point_color;
        }
        if (U_ChooseLight[0][3] == 1.0f) {
            theory_color += spot_color;
        }
    }
    // Area Light
    if (U_ChooseLight[1][1] == 1.0f) {
        area_color = cylinder_color;
        if (U_ChooseLight[1][1] == 1.0f) {
            area_color += rect_color;
        }
        if (U_ChooseLight[1][2] == 1.0f) {
            area_color += cylinder_color;
        }
        if (U_ChooseLight[1][3] == 1.0f) {
            area_color += sphere_color;
        }
    }

    color = theory_color + area_color;
    FragColor = color;
    HdrColor = color;
}

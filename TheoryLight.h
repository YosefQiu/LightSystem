#ifndef THEORYLIGHT_H
#define THEORYLIGHT_H

struct DLight
{
	vec4 U_Ambient;
	vec4 U_LightColor;
	vec4 U_LightDir;
};

struct PLight {
	vec4 U_Ambient;
	vec4 U_LightColor;
	vec4 U_LightPos;
};

struct FLight {
	vec4 U_Ambient;
	vec4 U_LightColor;
	vec4 U_LightPos;
	vec4 U_LightDir;
	vec4 U_LightOpt;
};

uniform DLight U_DLight;
uniform PLight U_PLight;
uniform FLight U_FLight;

vec4 CalcDirLight(DLight light, vec3 WorldPos, vec3 Normal, Material mat) {
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	vec3 L = normalize(-light.U_LightDir.xyz);
	vec3 N = normalize(Normal.xyz);
	vec3 V = normalize(U_CameraPos.xyz - WorldPos.xyz);
	vec3 H = normalize(L + V);

	float diffuseIntensity = max(0.0, dot(L, N));
	vec4 diffuseColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	if (U_bTexture) {
		diffuseColor = light.U_LightColor * mat.U_Diffuse * diffuseIntensity * texture(U_Texture, V_Texcoord.xy);
	}
	else
		diffuseColor = light.U_LightColor * mat.U_Diffuse * diffuseIntensity;

	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);

	if (diffuseIntensity > 0.0) {
		vec3 reflectDir = normalize(reflect(-L, N));
		vec3 worldPos = WorldPos.xyz;
		vec3 viewDir = normalize(U_CameraPos.xyz - worldPos.xyz);
		specularColor = light.U_LightColor * mat.U_Specular * pow(max(0.0, dot(N, H)), mat.U_Shininess);
	}
	vec4 ambientColor = light.U_Ambient * mat.U_Ambient;
	color = ambientColor + diffuseColor + specularColor;
	if (U_ChooseLightShadow[0][1] == 1.0f) {
		color = color * vec4(vec3(1.0 - CalcShadow()), 1.0);
	}

	return ToLinear(color);
}

vec4 CalcPointLight(PLight light, vec3 WorldPos, vec3 Normal, Material mat) {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    float dis = 0.0;
    float attenuation = 1.0;
    float constanfFactor = 0.5;
    float linearFactor = 0.0;
    float expFactor = 0.0;

    vec3 L = vec3(0.0, 0.0, 0.0);
    L = normalize(light.U_LightPos.xyz - WorldPos.xyz);
    vec3 N = normalize(Normal.xyz);
    dis = length(L);
    attenuation = 1.0 / (constanfFactor + linearFactor * dis + expFactor * expFactor * dis);

    float diffuseIntensity = max(0.0, dot(L, N));
    vec4 diffuseColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    if (U_bTexture) {
        diffuseColor = light.U_LightColor * mat.U_Diffuse * diffuseIntensity * attenuation * texture(U_Texture, V_Texcoord.xy);
    }
    else
        diffuseColor = light.U_LightColor * mat.U_Diffuse * diffuseIntensity;
    vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);

    if (diffuseIntensity > 0.0) {
        vec3 reflectDir = normalize(reflect(-L, N));
        vec3 worldPos = WorldPos.xyz;
        vec3 viewDir = normalize(U_CameraPos.xyz - worldPos.xyz);
        specularColor = light.U_LightColor * mat.U_Specular * pow(max(0.0, dot(viewDir, reflectDir)), mat.U_Shininess);
    }
    vec4 ambientColor = light.U_Ambient * mat.U_Ambient;
    color = ambientColor + (diffuseColor + specularColor) * attenuation;
    if (U_ChooseLightShadow[0][2] == 1.0f) {
        color = color * vec4(vec3(1.0 - CalcShadow()), 1.0);
    }
    return ToLinear(color);
}

vec4 CalcSpotLight(FLight light, vec3 WorldPos, vec3 Normal, Material mat) {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    float dis = 0.0;
    float attenuation = 1.0;
    float constanfFactor = 0.5;
    float linearFactor = 0.3;
    float expFactor = 0.1;

    vec3 L = vec3(0.0, 0.0, 0.0);
    L = normalize(light.U_LightPos.xyz - WorldPos.xyz);
    vec3 N = normalize(Normal.xyz);
    dis = length(L);
    attenuation = 1.0 / (constanfFactor + linearFactor * dis + expFactor * dis * dis);

    float diffuseIntensity = max(0.0, dot(L, N));
    vec4 diffuseColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);


    if (diffuseIntensity > 0.0) {
        vec3 spotDir = normalize(light.U_LightDir.xyz);
        float currentCosTheta = max(0.0, dot(-L, spotDir));
        float radianCutoff = light.U_LightDir.w * 3.14 / 180.0;
        float cosTheta = cos(radianCutoff);
        if (currentCosTheta > cosTheta) {
            diffuseIntensity = pow(currentCosTheta, light.U_LightOpt.x) * light.U_LightOpt.y;
        }
        else {
            diffuseIntensity = 0.0;
        }
    }
    if (diffuseIntensity > 0.0) {
        vec3 reflectDir = normalize(reflect(-L, N));
        vec3 worldPos = WorldPos.xyz;
        vec3 viewDir = normalize(U_CameraPos.xyz - worldPos.xyz);
        specularColor = light.U_LightColor * mat.U_Specular * pow(max(0.0, dot(viewDir, reflectDir)), mat.U_Shininess);
    }

    diffuseColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    if (U_bTexture) {
        diffuseColor = light.U_LightColor * mat.U_Diffuse * diffuseIntensity * attenuation * texture(U_Texture, V_Texcoord.xy);
    }
    else
        diffuseColor = light.U_LightColor * mat.U_Diffuse * diffuseIntensity * attenuation;
    vec4 ambientColor = light.U_Ambient * mat.U_Ambient;
    color = ambientColor + diffuseColor + specularColor;

    if (U_ChooseLightShadow[0][3] == 1.0f) {
        if (CalcShadow() > 0.0) {
            color = vec4(vec3(0.0), 1.0);
        }
    }
    return ToLinear(color);
}

#endif // THEORYLIGHT_H

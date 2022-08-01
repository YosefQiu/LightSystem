#ifndef SHADOW_H
#define SHADOW_H
uniform sampler2D U_ShadowMap;


float CalcShadow() {
	vec3 fragPos = V_LightSpaceFragPos.xyz / V_LightSpaceFragPos.w;
	fragPos = fragPos * 0.5 + vec3(0.5);
	float depthInShadowMap = texture(U_ShadowMap, fragPos.xy).r;
	float currentDepth = fragPos.z;
	vec2 texelSize = vec2(1.0 / 200.0, 1.0 / 200.0);
	float shadow = 0.0;
	for (int y = -1; y < 1; y++) {
		for (int x = -1; x < 1; x++) {
			float pcfDepth = texture(U_ShadowMap, fragPos.xy + texelSize * vec2(x, y)).r;
			shadow += (currentDepth - 0.001) > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	return shadow;
}


#endif // SHADOW_H

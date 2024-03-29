#version 450

layout (set = 1, binding = 0) uniform sampler2D uTexture;

layout (location = 0) in vec3 inColr;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inPosWorld;
layout (location = 3) in vec3 inNrmlWorld;

layout (location = 0) out vec4 outColor;

struct Light {
  vec4 pos;
  vec4 col;
  vec4 ambient;
};

layout (std140,set = 2, binding = 0) readonly buffer LightBuffer{
	int lightsCount;
	Light lights[];
} lightBuffer;

void main() {
  vec3 diffuse = vec3(1.0);
  vec3 surfaceNormal = normalize(inNrmlWorld);
  if (lightBuffer.lightsCount > 0) diffuse = lightBuffer.lights[0].ambient.xyz * lightBuffer.lights[0].ambient.w;
  for (int i = 0; i < lightBuffer.lightsCount; ++i) {
    Light light = lightBuffer.lights[i];
    vec3 dirToLight = light.pos.xyz - inPosWorld;
    float attenuation = 1.0 / dot(dirToLight, dirToLight);
    float cosAngIncidence = max(dot(surfaceNormal, normalize(dirToLight)), 0);
    vec3 intensity = light.col.xyz * light.col.w * attenuation;
    diffuse += intensity * cosAngIncidence; 
  }

  outColor = vec4(diffuse * (inColr * texture(uTexture, inUV).rgb), 1.0);
}
#version 450

layout(binding = 0) uniform CameraUBO {
  mat4 proj;
  mat4 view;
  vec4 pos;
} cam;

layout (set = 1, binding = 0) uniform sampler2D uAlbedo;
layout (set = 1, binding = 1) uniform sampler2D uNormal;
layout (set = 1, binding = 2) uniform sampler2D uMetallic;
layout (set = 1, binding = 3) uniform sampler2D uRoughness;
layout (set = 1, binding = 4) uniform sampler2D uAmbientOcclusion;

layout (location = 0) in vec3 inColr;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inPosWorld;
layout (location = 4) in vec3 inNormal;

layout (location = 0) out vec4 outColor;

struct Light {
  vec4 pos;
  vec4 col;
};

layout (std140,set = 2, binding = 0) readonly buffer LightBuffer{
	int lightsCount;
  vec4 ambient;
	Light lights[];
} lightBuffer;

const float PI = 3.14159265359;

vec3 getNormalFromMap();
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

//========================[Main]========================//
void main() {
  vec3  albedo    = pow(texture(uAlbedo, inUV).xyz, vec3(2.2));
  float metallic  = texture(uMetallic, inUV).r;
  float roughness = texture(uRoughness, inUV).r;
  float ao        = texture(uAmbientOcclusion, inUV).r;
  vec3  N         = getNormalFromMap();
  vec3  V         = normalize(cam.pos.xyz - inPosWorld);

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  vec3 Lo = vec3(0.0);
  for (int i = 0; i < lightBuffer.lightsCount; ++i) {
    Light light = lightBuffer.lights[i];
    light.col = vec4(light.col.xyz * light.col.w, 0.0);
    vec3 L = normalize(light.pos.xyz - inPosWorld);
    vec3 H = normalize(V + L);
    float distance = length(light.pos.xyz - inPosWorld);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.col.xyz * attenuation;

    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);

    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  vec3 ambient = lightBuffer.ambient.xyz * lightBuffer.ambient.w;
  ambient *= albedo;
  ambient *= ao;

  vec3 color = ambient + Lo;
  outColor = vec4(color, 1.0);
}
//========================[Main]========================//

vec3 getNormalFromMap() {
  vec3 tangentNormal = texture(uNormal, inUV).rgb * 2.0 - 1.0;

  vec3 Q1  = dFdx(inPosWorld);
  vec3 Q2  = dFdy(inPosWorld);
  vec2 st1 = dFdx(inUV);
  vec2 st2 = dFdy(inUV);

  vec3 N   =  normalize(inNormal);
  vec3 T   =  normalize(Q1*st2.t - Q2*st1.t);
  vec3 B   = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);

  return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness*roughness;
  float a2 = a*a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float nom   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r*r) / 8.0;

  float nom   = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
#version 450

layout(binding = 0) uniform CameraUBO {
  mat4 proj;
  mat4 view;
} cam;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColr;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inNrml;

layout (location = 0) out vec3 outColr;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outPosWorld;
layout (location = 3) out vec3 outNrmlWorld;

struct Light {
  vec4 pos;
  vec4 col;
  vec4 ambient;
};

layout (std140,set = 2, binding = 0) readonly buffer LightBuffer{
	int lightsCount;
	Light lights[];
} lightBuffer;

layout( push_constant ) uniform constants {
  mat4 model;
  mat4 normal;
} pc;

void main() {
  vec4 worldPos = pc.model * vec4(inPos, 1.0);
  gl_Position = cam.proj * cam.view * worldPos;
  gl_Position.y = -gl_Position.y;
  
  outColr = inColr;
  outUV   = inUV;
  outPosWorld = worldPos.xyz;
  outNrmlWorld = normalize(mat3(pc.normal) * inNrml);
}
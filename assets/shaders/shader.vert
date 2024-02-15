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
layout (location = 2) out vec3 outNrml;

void main() {
  gl_Position = cam.proj * cam.view * vec4(inPos, 1.0);
  outColr     = inColr;
  outUV       = inUV;
  outNrml     = inNrml;
}
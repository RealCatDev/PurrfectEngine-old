#version 450

layout(binding = 0) uniform CameraUBO {
  mat4 proj;
  mat4 view;
  vec4 pos;
} cam;

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3 outPos;

void main() {
  outPos = inPos;
  gl_Position = cam.proj * cam.view * vec4(outPos, 1.0);
  gl_Position.y = -gl_Position.y;
}
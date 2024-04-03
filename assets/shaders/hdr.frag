#version 450

layout (location = 0) in  vec2 inUV;
layout (location = 0) out vec4 outColor;

layout(binding = 0) uniform CameraUBO {
  mat4 proj;
  mat4 view;
  vec4 pos; // w = exposure
} cam;

layout(set = 1, binding = 0) uniform sampler2D uTexture;

void main() {
  vec3 hdrColor = texture(uTexture, inUV).rgb;
  const float gamma = 2.2;
  vec3 result = vec3(1.0) - exp(-hdrColor * cam.pos.w);
  result = pow(result, vec3(1.0 / gamma));
  outColor = vec4(result, 1.0);
}
#version 450

layout(binding = 0) uniform sampler2D uTexture;

layout (location = 0) in  vec2 inUV;
layout (location = 0) out vec4 outColor;

void main() {
  const float gamma = 2.2;
  vec3 color = texture(uTexture, inUV).rgb;
  vec3 mapped = color / (color + vec3(1.0));
  mapped = pow(mapped, vec3(1.0 / gamma));
  outColor = vec4(mapped, 1.0);
}
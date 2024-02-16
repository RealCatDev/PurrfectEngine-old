#version 450

layout(set = 1, binding = 0) uniform sampler2D uTexture;

layout (location = 0) in vec3 inColr;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNrml;

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(inColr, 1.0) * texture(uTexture, inUV);
}
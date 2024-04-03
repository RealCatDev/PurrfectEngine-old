#version 450

layout (set = 1, binding = 0) uniform sampler2D uEquirectangularMap;

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec4 outColor;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 vert) {
  vec2 uv = vec2(atan(vert.z, vert.x), asin(vert.y));
  uv *= invAtan;
  return uv + 0.5;
}

void main() {
  vec2 uv = SampleSphericalMap(normalize(inPos));
  vec3 color = texture(uEquirectangularMap, uv).rgb;
  outColor = vec4(color, 1.0);
}
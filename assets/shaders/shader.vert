#version 450

layout (location = 0) vec3 inPos;
layout (location = 1) vec3 inColr;
layout (location = 2) vec2 inUV;
layout (location = 3) vec3 inNrml;

layout (location = 0) vec3 outColr;
layout (location = 1) vec2 outUV;
layout (location = 2) vec3 outNrml;

void main() {
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
#version 150

uniform mat4 PVMmatrix;
in vec2 in_Position;
out vec3 texCoord_v;

void main() {
  vec4 farplaneCoord = vec4(in_Position, 0.9999, 1.0);
  vec4 worldViewCoord = PVMmatrix * farplaneCoord;
  texCoord_v = worldViewCoord.xyz / worldViewCoord.w;
  gl_Position = farplaneCoord;
}
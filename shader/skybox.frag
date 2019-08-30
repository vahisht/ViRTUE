#version 150

precision highp float;

uniform samplerCube skyboxSampler;
in vec3 texCoord_v;
out vec4 fragColor;

void main() {
  fragColor = texture(skyboxSampler, texCoord_v);
}
#version 330 core

uniform mat4 model;

layout (location = 0) in vec3 position;

void main() {
  gl_Position = model * vec4(position, 1.0);
}

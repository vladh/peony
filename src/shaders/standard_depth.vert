uniform mat4 model_matrix;

layout (location = 0) in vec3 position;

void main() {
  gl_Position = model_matrix * vec4(position, 1.0);
}

uniform mat4 model_matrix;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out BLOCK {
  vec3 normal;
} vs_out;

void main() {
  gl_Position = projection * view * model_matrix * vec4(position, 1.0);
  vs_out.normal = normalize(normal);
}

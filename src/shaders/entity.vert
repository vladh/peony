uniform mat4 model;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out VS_OUT {
  vec3 frag_position;
  vec3 normal;
  vec2 tex_coords;
} vs_out;

void main() {
  vs_out.frag_position = vec3(model * vec4(position, 1.0));
  vs_out.normal = mat3(transpose(inverse(model))) * normal;
  vs_out.tex_coords = tex_coords;
  gl_Position = projection * view * vec4(vs_out.frag_position, 1.0);
}

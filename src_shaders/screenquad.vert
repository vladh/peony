layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out BLOCK {
  vec2 tex_coords;
} vs_out;

void main() {
  vs_out.tex_coords = tex_coords;
  gl_Position = vec4(position, 1.0);
}

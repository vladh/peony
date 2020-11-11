layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

out vec2 tex_coords;

void main() {
  gl_Position = ui_projection * vec4(vertex.xy, 0.0, 1.0);
  tex_coords = vertex.zw;
}

#define N_VERTICES 3

layout (triangles) in;
layout (triangle_strip, max_vertices=N_VERTICES) out;

out VS_OUT {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} gs_in;

out GS_OUT {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} gs_out;

void main() {
  for (int idx = 0; idx < N_VERTICES; idx++) {
    gs_out.world_position = gs_in.world_position;
    gl_Position = gl_in[idx].gl_Position;
    gs_out.normal = gs_in.normal;
    gs_out.tex_coords = gs_in.tex_coords;

    EmitVertex();
  }

  EndPrimitive();
}

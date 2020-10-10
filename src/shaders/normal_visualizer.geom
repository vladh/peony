layout (triangles) in;
layout (line_strip, max_vertices=10) out;

in BLOCK {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} gs_in[3];

out BLOCK {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} gs_out;

void main() {
  float normal_length = 0.2;

  for (int idx = 0; idx < 3; idx++) {
    vec3 P = gs_in[idx].world_position.xyz;
    vec3 N = gs_in[idx].normal.xyz;

    gl_Position = projection * view * vec4(P, 1.0);
    gs_out.world_position = gs_in[idx].world_position;
    gs_out.normal = gs_in[idx].normal;
    gs_out.tex_coords = gs_in[idx].tex_coords;
    EmitVertex();

    gl_Position = projection * view * vec4(P + N * normal_length, 1.0);
    gs_out.normal = vec3(0.0, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
  }

  for (int idx = 0; idx < 4; idx++) {
    vec3 P = gs_in[idx % 3].world_position.xyz;
    vec3 N = gs_in[idx % 3].normal.xyz;

    gl_Position = projection * view * vec4(P, 1.0);
    gs_out.world_position = gs_in[idx % 3].world_position;
    gs_out.normal = gs_in[idx % 3].normal;
    gs_out.tex_coords = gs_in[idx % 3].tex_coords;
    EmitVertex();
  }
  EndPrimitive();
}

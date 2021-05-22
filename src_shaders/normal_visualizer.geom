layout (triangles) in;
layout (line_strip, max_vertices=32) out;

in BLOCK {
  vec3 world_position;
  vec2 screen_position;
  vec3 normal;
  vec3 bitangent;
  vec3 tangent;
  vec2 tex_coords;
} gs_in[3];

out BLOCK {
  vec3 world_position;
  vec2 screen_position;
  vec3 normal;
  vec3 bitangent;
  vec3 tangent;
  vec2 tex_coords;
} gs_out;

void main() {
  float normal_length = 0.1;

  for (int idx = 0; idx < 3; idx++) {
    vec3 P = gs_in[idx].world_position.xyz;
    vec3 N = gs_in[idx].normal.xyz;

    gl_Position = projection * view * vec4(P, 1.0);
    gs_out.world_position = gs_in[idx].world_position;
    gs_out.screen_position = gs_in[idx].screen_position;
    gs_out.normal = gs_in[idx].normal;
    gs_out.bitangent = gs_in[idx].bitangent;
    gs_out.tangent = gs_in[idx].tangent;
    gs_out.tex_coords = gs_in[idx].tex_coords;
    EmitVertex();

    gl_Position = projection * view * vec4(P + N * normal_length, 1.0);
    gs_out.normal = vec3(0.0, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
  }

  for (int idx = 0; idx < 3; idx++) {
    vec3 P = gs_in[idx].world_position.xyz;
    vec3 N = gs_in[idx].tangent.xyz;

    gl_Position = projection * view * vec4(P, 1.0);
    gs_out.world_position = gs_in[idx].world_position;
    gs_out.screen_position = gs_in[idx].screen_position;
    gs_out.normal = gs_in[idx].normal;
    gs_out.bitangent = gs_in[idx].bitangent;
    gs_out.tangent = gs_in[idx].tangent;
    gs_out.tex_coords = gs_in[idx].tex_coords;
    EmitVertex();

    gl_Position = projection * view * vec4(P + N * normal_length, 1.0);
    gs_out.normal = vec3(0.0, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
  }

  for (int idx = 0; idx < 3; idx++) {
    vec3 P = gs_in[idx].world_position.xyz;
    vec3 N = gs_in[idx].bitangent.xyz;

    gl_Position = projection * view * vec4(P, 1.0);
    gs_out.world_position = gs_in[idx].world_position;
    gs_out.screen_position = gs_in[idx].screen_position;
    gs_out.normal = gs_in[idx].normal;
    gs_out.bitangent = gs_in[idx].bitangent;
    gs_out.tangent = gs_in[idx].tangent;
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
    gs_out.screen_position = gs_in[idx % 3].screen_position;
    gs_out.normal = gs_in[idx % 3].normal;
    gs_out.bitangent = gs_in[idx % 3].bitangent;
    gs_out.tangent = gs_in[idx % 3].tangent;
    gs_out.tex_coords = gs_in[idx % 3].tex_coords;
    EmitVertex();
  }
  EndPrimitive();
}

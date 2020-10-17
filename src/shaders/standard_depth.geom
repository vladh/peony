layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

out BLOCK {
  vec4 world_position;
} gs_out;

void main() {
  for (int face = 0; face < 6; face++) {
    int layer_face = (shadow_light_idx * 6) + face;
    gl_Layer = layer_face;

    for (int idx = 0; idx < 3; idx++) {
      gs_out.world_position = gl_in[idx].gl_Position;
      gl_Position = shadow_transforms[layer_face] * gs_out.world_position;
      EmitVertex();
    }

    EndPrimitive();
  }
}

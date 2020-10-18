layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

out BLOCK {
  vec4 world_position;
} gs_out;

void main() {
  int n_faces_for_this_light;
  if (light_type[shadow_light_idx].x == LIGHT_POINT) {
    n_faces_for_this_light = 6;
  } else if (light_type[shadow_light_idx].x == LIGHT_DIRECTIONAL) {
    n_faces_for_this_light = 1;
  }

  int layer_face;
  for (int face = 0; face < n_faces_for_this_light; face++) {
    int layer_face = (shadow_light_idx * 6) + face;
    int render_layer;
    if (light_type[shadow_light_idx].x == LIGHT_POINT) {
      render_layer = layer_face;
    } else if (light_type[shadow_light_idx].x == LIGHT_DIRECTIONAL) {
      render_layer = shadow_light_idx + face;
    }
    gl_Layer = render_layer;

    for (int idx = 0; idx < 3; idx++) {
      gs_out.world_position = gl_in[idx].gl_Position;
      gl_Position = shadow_transforms[layer_face] * gs_out.world_position;
      EmitVertex();
    }

    EndPrimitive();
  }
}

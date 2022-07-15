// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

out BLOCK {
  vec4 world_position;
} gs_out;

void main() {
  int n_faces_for_this_light;
  if (current_shadow_light_type == LIGHT_POINT) {
    n_faces_for_this_light = 6;
  } else if (current_shadow_light_type == LIGHT_DIRECTIONAL) {
    n_faces_for_this_light = 1;
  }

  for (int face = 0; face < n_faces_for_this_light; face++) {
    int layer_face = (current_shadow_light_idx * n_faces_for_this_light) + face;
    gl_Layer = layer_face;

    for (int idx = 0; idx < 3; idx++) {
      gs_out.world_position = gl_in[idx].gl_Position;

      mat4 shadow_transform;
      if (current_shadow_light_type == LIGHT_POINT) {
        shadow_transform = shadowmap_3d_transforms[layer_face];
      } else if (current_shadow_light_type == LIGHT_DIRECTIONAL) {
        shadow_transform = shadowmap_2d_transforms[layer_face];
      }

      gl_Position = shadow_transform * gs_out.world_position;
      EmitVertex();
    }

    EndPrimitive();
  }
}

uniform mat4 model_matrix;
uniform mat3 model_normal_matrix;
uniform mat4 bone_matrices[MAX_N_BONES];

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;
layout (location = 3) in ivec4 bone_idxs;
layout (location = 4) in vec4 bone_weights;

out BLOCK {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} vs_out;

void main() {
  mat4 bone_transform = mat4(0.0);
  bool did_find_bones = false;

  for (int idx = 0; idx < 4; idx++) {
    if (bone_idxs[idx] > 0) {
      did_find_bones = true;
      bone_transform += bone_matrices[bone_idxs[idx]] * bone_weights[idx];
    }
  }

  if (did_find_bones) {
    vs_out.world_position = vec3(bone_transform * model_matrix * vec4(position, 1.0));
    vs_out.normal = normalize(mat3(bone_transform) * model_normal_matrix * normal);
  } else {
    vs_out.world_position = vec3(model_matrix * vec4(position, 1.0));
    vs_out.normal = normalize(model_normal_matrix * normal);
  }
  vs_out.tex_coords = tex_coords;
  gl_Position = projection * view * vec4(vs_out.world_position, 1.0);
}

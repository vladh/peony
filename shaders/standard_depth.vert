uniform mat4 model_matrix;
uniform mat4 bone_matrices[MAX_N_BONES];

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;
layout (location = 3) in uvec4 bone_idxs;
layout (location = 4) in vec4 bone_weights;

void main() {
  mat4 bone_transform = mat4(0.0);
  bool did_find_bones = false;

  for (int idx = 0; idx < 4; idx++) {
    if (bone_weights[idx] > 0) {
      did_find_bones = true;
      bone_transform += bone_matrices[bone_idxs[idx]] * bone_weights[idx];
    }
  }

  if (!did_find_bones) {
    bone_transform = mat4(1.0f);
  }

  gl_Position = model_matrix * bone_transform * vec4(position, 1.0);
}

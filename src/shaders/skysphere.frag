out vec4 frag_color;

in BLOCK {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} fs_in;

void main() {
  vec3 N = normalize(fs_in.normal);
  vec3 color = vec3(1.0);
  vec3 origin = vec3(camera_position);
  vec3 sun_pos = -SUN_DIRECTION;
  float sun_dist = length(fs_in.world_position - sun_pos);
  float pos_dot_y_axis = abs(dot(normalize(fs_in.world_position), vec3(0.0, 1.0, 0.0)));

  if (sun_dist < SUN_SIZE) {
    // Actual sun blob.
    color = SUN_ALBEDO;
  } else {
    // Sky color.
    color = mix(GROUND_ALBEDO, SKY_ALBEDO, pos_dot_y_axis);
  }

  frag_color = vec4(color, 1.0f);
}

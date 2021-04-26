in BLOCK {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} fs_in;

layout (location = 0) out vec4 l_color;
layout (location = 1) out vec4 l_bright_color;

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

  l_color = vec4(color, 1.0);
  float brightness = dot(l_color.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 1.0) {
    l_bright_color = l_color;
  } else {
    l_bright_color = vec4(0.0, 0.0, 0.0, 1.0);
  }
}

out vec4 frag_color;

void main() {
  frag_color = vec4(1.0f, 0.0f, 1.0f, 1.0f);

  // // Pixels which have no normals are background pixels.
  // if (normal == vec3(0.0, 0.0, 0.0)) {
  //   float angle = normalize_angle(
  //     (-camera_pitch) +
  //     ((fs_in.tex_coords.y - 0.5) * (camera_vertical_fov * 2.0))
  //   );
  //   float angle_rad = radians(angle);

  //   if (abs(angle_rad - SUN_ANGLE) < 0.1) {
  //     // Actual sun blob.
  //     frag_color = vec4(1.0, 0.0, 0.0, 0.0);
  //   } else {
  //     // Sky color.
  //     frag_color = vec4(get_sky_color(angle), 1.0);
  //   }
  //   return;
  // }
}

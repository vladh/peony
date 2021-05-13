uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D g_albedo_texture;
uniform sampler2D g_pbr_texture;

uniform sampler2D l_color_texture;
uniform sampler2D l_bright_color_texture;
uniform sampler2D l_depth_texture;

uniform sampler2D blur1_texture;
uniform sampler2D blur2_texture;

in BLOCK {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

void main() {
  if (renderdebug_displayed_texture_type == TEXTURETYPE_NONE) {
    discard;
  }

  float preview_screen_proportion = 1.0;
  float aspect_ratio = window_width / window_height;
  vec2 preview_size = vec2(
    preview_screen_proportion,
    preview_screen_proportion / aspect_ratio
  );

  if (
    fs_in.tex_coords.x < preview_size.x &&
    fs_in.tex_coords.y < preview_size.y
  ) {
    vec2 preview_tex_coords = fs_in.tex_coords / preview_size;
    vec2 sample_p = preview_tex_coords;

    vec3 color;
    if (renderdebug_displayed_texture_type == TEXTURETYPE_G_POSITION) {
      color = vec3(texture(g_position_texture, sample_p));
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_G_NORMAL) {
      color = vec3(texture(g_normal_texture, sample_p));
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_G_ALBEDO) {
      color = vec3(texture(g_albedo_texture, sample_p));
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_G_PBR) {
      color = vec3(texture(g_pbr_texture, sample_p));
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_L_COLOR) {
      color = vec3(texture(l_color_texture, sample_p));
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_L_BRIGHT_COLOR) {
      color = vec3(texture(l_bright_color_texture, sample_p));
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_L_DEPTH) {
      color = vec3(texture(l_depth_texture, sample_p));
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_BLUR1) {
      color = vec3(texture(blur1_texture, sample_p));
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_BLUR2) {
      color = vec3(texture(blur2_texture, sample_p));
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_SHADOWMAPS_3D) {
      color = texture(shadowmaps_3d, vec4(sample_p, 0.0, 0.0)).rrr;
    } else if (renderdebug_displayed_texture_type == TEXTURETYPE_SHADOWMAPS_2D) {
      color = texture(shadowmaps_2d, vec3(sample_p, 0.0)).rrr;
    }

    frag_color = vec4(color, 1.0);
  } else {
    discard;
  }
}

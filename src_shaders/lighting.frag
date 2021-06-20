/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D g_albedo_texture;
uniform sampler2D g_pbr_texture;

in BLOCK {
  vec2 tex_coords;
} fs_in;

layout (location = 0) out vec4 l_color;
layout (location = 1) out vec4 l_bright_color;

void main() {
  vec3 world_position = texture(g_position_texture, fs_in.tex_coords).rgb;
  vec3 normal = texture(g_normal_texture, fs_in.tex_coords).rgb;
  vec3 albedo = linearize_albedo(texture(g_albedo_texture, fs_in.tex_coords).rgb);
  vec4 pbr_texture = texture(g_pbr_texture, fs_in.tex_coords);
  float metallic = pbr_texture.r;
  float roughness = pbr_texture.g;
  float ao = pbr_texture.b;

  vec3 color = compute_pbr_light(
    albedo, metallic, roughness, ao,
    world_position, normal
  );

  l_color = vec4(color, 1.0);
  float brightness = dot(l_color.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 1.0) {
    l_bright_color = l_color;
  } else {
    l_bright_color = vec4(0.0, 0.0, 0.0, 1.0);
  }
}

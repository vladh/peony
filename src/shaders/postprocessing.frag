#version 330 core

#define MAX_N_LIGHTS 32
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS
#define MAX_N_TEXTURES 32

#define GAMMA 2.2

uniform int n_diffuse_textures;
uniform sampler2D diffuse_textures[MAX_N_TEXTURES];
uniform int n_specular_textures;
uniform sampler2D specular_textures[MAX_N_TEXTURES];
uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

uniform float exposure;

in VS_OUT {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

vec3 invert(vec3 rgb) {
  return 1 - rgb;
}

vec3 correct_gamma(vec3 rgb) {
  return pow(rgb, vec3(1.0 / GAMMA));
}

vec3 add_tone_mapping(vec3 rgb) {
  // Reinhard tone mapping
  /* return rgb / (rgb + vec3(1.0)); */
  // Exposure tone mapping
  return vec3(1.0) - exp(-rgb * exposure);
}

void main() {
  vec3 rgb = texture(diffuse_textures[0], fs_in.tex_coords).rgb;

  rgb = add_tone_mapping(rgb);
  rgb = correct_gamma(rgb);
  /* rgb = invert(rgb); */

  frag_color = vec4(rgb, 1.0f);
}

#version 330 core

#define MAX_N_LIGHTS 32
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS
#define MAX_N_TEXTURES 32

uniform int n_diffuse_textures;
uniform sampler2D diffuse_textures[MAX_N_TEXTURES];
uniform int n_specular_textures;
uniform sampler2D specular_textures[MAX_N_TEXTURES];
uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

in VS_OUT {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

void main() {
  frag_color = vec4(
    1.0f - texture(diffuse_textures[0], fs_in.tex_coords).rgb,
    1.0f
  );
}

uniform int n_diffuse_textures;
uniform sampler2D diffuse_textures[MAX_N_TEXTURES];
uniform int n_specular_textures;
uniform sampler2D specular_textures[MAX_N_TEXTURES];
uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

in VS_OUT {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

void main() {
  // retrieve data from G-buffer
  vec3 frag_position = texture(diffuse_textures[0], fs_in.tex_coords).rgb;
  vec3 normal = texture(diffuse_textures[1], fs_in.tex_coords).rgb;
  vec3 diffuse_texture_0 = texture(diffuse_textures[2], fs_in.tex_coords).rgb;
  float specular_texture_0 = texture(diffuse_textures[2], fs_in.tex_coords).a;

  vec3 N = normalize(normal);
  vec3 V = normalize(camera_position - frag_position);

  vec3 Lo = vec3(0.0);

  for (int idx_light = 0; idx_light < n_lights; idx_light++) {
    Light light = lights[idx_light];

    vec3 L = normalize(light.position - frag_position);
    vec3 H = normalize(V + L);

    float distance = length(light.position - frag_position);
    float attenuation = 1.0 / (
      light.attenuation_constant +
      light.attenuation_linear * distance +
      light.attenuation_quadratic * (distance * distance)
    );
    vec3 radiance = light.color * attenuation;
  }

  frag_color = vec4(lighting, 1.0f);
}

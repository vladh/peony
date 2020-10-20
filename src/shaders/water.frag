vec3 SKY_ALBEDO = vec3(0.81, 0.93, 1.00);
vec3 WATER_ALBEDO_DEEP = vec3(0.00, 0.09, 0.18);
// vec3 WATER_ALBEDO_SHALLOW = vec3(0.0, 0.5, 0.5);
vec3 WATER_ALBEDO_SHALLOW = vec3(0.48, 0.54, 0.36);
float WATER_MAX_DEPTH = 2.0;
float WATER_MIN_DEPTH = 0.0;
float WATER_MAX_PASSTHROUGH_DISTANCE = 2.0;

uniform sampler2D g_position_texture;
uniform sampler2D g_albedo_texture;

in BLOCK {
  vec3 world_position;
  vec2 screen_position;
  vec3 normal;
  vec2 tex_coords;
} fs_in;

layout (location = 0) out vec4 frag_color;


float diffuse(vec3 N, vec3 L, float p) {
  return pow(dot(N, L) * 0.4 + 0.6, p);
}


float specular(vec3 N, vec3 L, vec3 V, float s) {
  float nrm = (s + 8.0) / (M_PI * 8.0);
  return pow(max(dot(reflect(V, N), L), 0.0), s) * nrm;
}


void main() {
  vec3 curr_light_position = vec3(light_position[0]); // Directional light
  vec3 unit_normal = normalize(fs_in.normal);
  vec3 N = unit_normal;
  vec3 V = normalize(camera_position - fs_in.world_position);
  vec3 L = normalize(curr_light_position - fs_in.world_position);
  vec3 H = normalize(V + L);
  float h_dot_v = max(dot(H, V), M_EPSILON);

  // float F = fresnel_schlick(h_dot_v, WATER_F0);
  float F = pow(clamp(1.0 - h_dot_v, 0.0, 1.0), 3.0) * 0.5;

  vec3 water_albedo = mix(WATER_ALBEDO_DEEP, WATER_ALBEDO_SHALLOW, 1 - h_dot_v);
  vec3 underwater_albedo = texture(g_albedo_texture, fs_in.screen_position).rgb;
  vec3 underwater_object_world_position = texture(g_position_texture, fs_in.screen_position).rgb;
  float underwater_object_distance_factor = (1 - to_unit_interval(
    length(fs_in.world_position - underwater_object_world_position),
    0.0,
    WATER_MAX_PASSTHROUGH_DISTANCE
  )) / 2;

  // vec3 refraction_color = desaturate(underwater_albedo, 0.5) * water_albedo;
  // vec3 pbr_light = compute_pbr_light(
  //   water_albedo, 0.0, 1.0, 1.0,
  //   fs_in.world_position, fs_in.normal
  // );

  // vec3 refraction_color = mix(underwater_albedo, water_albedo, F);
  // frag_color = vec4(pbr_light + refraction_color, 1.0);
  // frag_color = vec4((pbr_light * 0.5) + refraction_color, 1.0);
  // frag_color = vec4(refraction_color, 1.0);
  // frag_color = vec4(F, F, F, 1.0);

  float depth_factor = to_unit_interval(fs_in.world_position.y, WATER_MIN_DEPTH, WATER_MAX_DEPTH);

  vec3 reflected = SKY_ALBEDO;
  vec3 refracted = 0 +
    WATER_ALBEDO_DEEP +
    diffuse(N, L, 80.0) * WATER_ALBEDO_SHALLOW * 0.12 +
    (underwater_albedo * underwater_object_distance_factor) +
    vec3(1.0, 0.0, 0.0) * depth_factor;
  vec3 color = mix(refracted, reflected, F);

  frag_color = vec4(color, 1.0);
}

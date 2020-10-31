#define N_UNDERWATER_POSITION_SAMPLES 1

vec3 WATER_ALBEDO_DEEP = vec3(0.00, 0.09, 0.18);
vec3 WATER_ALBEDO_SHALLOW = vec3(0.0, 0.5, 0.5);
vec3 WATER_FOAM_COLOR = vec3(1.0, 1.0, 1.0);
float WATER_FOAM_ALPHA = 0.8;
float WATER_MAX_DEPTH = 2.0;
float WATER_MIN_DEPTH = 0.0;
float WATER_MAX_PASSTHROUGH_DISTANCE = 1.5;
float WATER_FOAM_DIST = 1.5;

uniform sampler2D g_position_texture;
uniform sampler2D g_albedo_texture;
uniform sampler2D normal_texture;
uniform sampler2D foam_texture;

uniform bool should_use_normal_map;

layout (location = 0) out vec4 frag_color;

in BLOCK {
  vec3 world_position;
  vec2 screen_position;
  vec3 normal;
  vec3 bitangent;
  vec3 tangent;
  vec2 tex_coords;
} fs_in;


vec3 get_sky_color(vec3 V) {
  // TODO: Figure this out.
  float y = (max(V.y,0.0)*0.8+0.2)*0.8;
  return vec3(pow(1.0-y,2.0), 1.0-y, 0.6+(1.0-y)*0.4) * 1.1;
}


float diffuse(float n_dot_l, float p) {
  return pow(n_dot_l * 0.4 + 0.6, p);
}


float specular(float r_dot_l) {
  // float s = 60;
  // float nrm = (s + 8.0) / (M_PI * 8.0);
  // return pow(r_dot_l, s) * nrm;
  float s = 32;
  float specular_strength = 0.5;
  return pow(r_dot_l, s) * specular_strength;
}


void main() {
  vec3 underwater_position = texture(g_position_texture, fs_in.screen_position).rgb;
  vec3 curr_light_position = vec3(light_position[0]); // Directional light
  vec3 unit_normal = normalize(fs_in.normal);
  const float plane_size = 100.0;
  vec2 corrected_tex_coords = (fs_in.world_position.xz - plane_size / 2) / plane_size;
  vec2 normal_tex_coords = corrected_tex_coords * 10.0/* - vec2(t / 40, t / 40)*/;
  vec3 N;

  if (should_use_normal_map) {
    vec3 tangent_normal_rgb = texture(normal_texture, normal_tex_coords).xyz * 2.0 - 1.0;
    vec3 tangent_normal = vec3(
      tangent_normal_rgb.b, tangent_normal_rgb.g, tangent_normal_rgb.r
    );
    mat3 TBN = mat3(fs_in.tangent, fs_in.bitangent, fs_in.normal);
    N = normalize(TBN * tangent_normal);
  } else {
    N = unit_normal;
  }

  vec3 V = normalize(camera_position - fs_in.world_position);
  vec3 L = normalize(curr_light_position - fs_in.world_position);
  vec3 H = normalize(V + L);
  // vec3 R = reflect(-L, N);
  vec3 R = reflect(-V, N);
  float h_dot_v = max(dot(H, V), M_EPSILON);
  float n_dot_v = max(dot(N, V), M_EPSILON);
  float n_dot_l = max(dot(N, L), M_EPSILON);
  float r_dot_l = max(dot(R, L), M_EPSILON);
  vec3 n_min_v = N - V;

  // TODO: Figure out what's up here.
  float F = fresnel_schlick(h_dot_v, WATER_F0) * 9.0;
  // float F = pow(clamp(1.0 - dot(N, -V), 0.0, 1.0), 3.0) * 0.5;

  // NOTE: We sample our coordinates from the g_albedo_texture, with an offset
  // to account for refraction. However, because we're not taking depth
  // information in to account, this offset can lead us to sampling
  // an object that is _in front_ of our waves. Hence, we check what's farther,
  // the water, or the sampled "underwater" object. We allow for the object
  // to be within a distance of 1 closer to the camera than the water,
  // otherwise we discard the sampled albedo by setting it to 0.
  vec2 refraction_dir = normalize(vec2(n_min_v.z, n_min_v.x));
  vec2 refracted_tex_coords = fs_in.screen_position - refraction_dir * 0.010;
  vec3 refracted_underwater_position = texture(g_position_texture, refracted_tex_coords).rgb;
  float water_dist = length(camera_position - fs_in.world_position);
  float underwater_dist = length(camera_position - refracted_underwater_position);
  float sampled_albedo_discard_factor = max(0, 1 - ((water_dist - underwater_dist) / 1));
  float underwater_distance_factor = (1 - to_unit_interval(
    length(fs_in.world_position - underwater_position),
    0.0,
    WATER_MAX_PASSTHROUGH_DISTANCE
  )) / 2;
  vec3 underwater_albedo = texture(g_albedo_texture, refracted_tex_coords).rgb *
    sampled_albedo_discard_factor *
    underwater_distance_factor;

  float depth_factor = to_unit_interval(fs_in.world_position.y, WATER_MIN_DEPTH, WATER_MAX_DEPTH);

  vec3 reflected = get_sky_color(R);
  vec3 refracted = 0 +
    WATER_ALBEDO_DEEP +
    diffuse(n_dot_l, 80.0) * WATER_ALBEDO_SHALLOW * 0.12 +
    underwater_albedo;

  vec3 color = mix(refracted, reflected, F);
  color += WATER_ALBEDO_SHALLOW * depth_factor;
  color += specular(r_dot_l);

  // TODO: There's a better way of doing this.
  vec2 foam_tex_coords = corrected_tex_coords * 90.0;
  float channelA = texture(
    foam_texture,
    foam_tex_coords - vec2(3.0, cos(corrected_tex_coords.x))
  ).r;
  float channelB = texture(
    foam_texture,
    foam_tex_coords * 0.5 + vec2(sin(corrected_tex_coords.y), 3.0)
  ).b;
  float mask = clamp(pow((channelA + channelB), 2), 0, 1);

  float water_to_underwater_dist = length(fs_in.world_position - underwater_position);
  float foam_falloff = 1 - (water_to_underwater_dist / WATER_FOAM_DIST);
  vec3 foam = WATER_FOAM_COLOR * WATER_FOAM_ALPHA * foam_falloff;
  if (underwater_position != vec3(0.0, 0.0, 0.0)) {
    color += clamp(foam - mask, 0.0, 1.0);
  }

  // color = N;

  frag_color = vec4(color, 1.0);
}

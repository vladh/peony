// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#define N_UNDERWATER_POSITION_SAMPLES 1

#define ARE_NORMAL_MAPS_ENABLED true

// vec3 WATER_ALBEDO_DEEP = vec3(0.00, 0.09, 0.18);

vec3 WATER_ALBEDO_DEEP = vec3(0.00, 0.01, 0.10);
vec3 WATER_ALBEDO_SHALLOW = vec3(0.0, 0.5, 0.5);
vec3 WATER_ALBEDO_VERY_SHALLOW = vec3(0.0, 0.9, 0.9);
vec3 WATER_ALBEDO_BRIGHT = vec3(1.0, 1.0, 1.0);

// vec3 WATER_ALBEDO_SHALLOW = vec3(0.52, 0.77, 0.80);
// vec3 WATER_ALBEDO_DEEP = vec3(0.00, 0.44, 0.50);

vec3 WATER_FOAM_COLOR = vec3(1.0, 1.0, 1.0);
float WATER_FOAM_ALPHA = 0.8;
float WATER_MAX_HEIGHT = 6.0;
float WATER_MIN_HEIGHT = -0.5;
float WATER_MAX_PASSTHROUGH_DISTANCE = 1.5;
float WATER_FOAM_DIST = 1.5;
float WATER_FOAM_SKIRT_DIST = 0.5;
float WATER_SHADOW_FACTOR = 0.3;

uniform sampler2D g_position_texture;
uniform sampler2D g_albedo_texture;
uniform sampler2D normal_texture;
uniform sampler2D foam_texture;

uniform bool should_use_normal_map;

layout (location = 0) out vec4 l_color;
layout (location = 1) out vec4 l_bright_color;


in BLOCK {
  vec3 world_position;
  vec3 normal;
#if SHOULD_CALCULATE_TANGENT_IN_VERTEX_SHADER
  vec3 bitangent;
  vec3 tangent;
#endif
  vec2 tex_coords;
} fs_in;


float diffuse(float n_dot_l) {
  // return pow(n_dot_l * 0.4 + 0.3, 120.0);
  // return pow(n_dot_l * 0.3 + 0.1, 2.0);
  return pow(n_dot_l * 0.4 + 0.3, 4.0);
}


float specular(float r_dot_v) {
  // float s = 60;
  // float nrm = (s + 8.0) / (M_PI * 8.0);
  // return pow(r_dot_v, s) * nrm;
  return pow(r_dot_v, 1500) * 12.0;
}


void main() {
  const float plane_size = 100.0;
  vec2 corrected_tex_coords = (fs_in.world_position.xz - plane_size / 2) / plane_size;
  vec2 normal_tex_coords_1 = corrected_tex_coords * 10.0 /*- vec2(sin(t) / 25, cos(t) / 35*/;
  vec2 normal_tex_coords_2 = (corrected_tex_coords + vec2(0.2, 0.4)) * 15.0;
  vec3 unit_normal = normalize(fs_in.normal);
  vec3 N;

  if (ARE_NORMAL_MAPS_ENABLED && should_use_normal_map) {
#if SHOULD_CALCULATE_TANGENT_IN_VERTEX_SHADER
    // TODO: Fix or remove this. It's broken atm.
    vec3 tangent_normal_1_rgb = texture(normal_texture, normal_tex_coords_1).xyz * 2.0 - 1.0;
    vec3 tangent_normal_1 = vec3(
      tangent_normal_1_rgb.b, tangent_normal_1_rgb.g, tangent_normal_1_rgb.r
    );
    vec3 tangent_normal_2_rgb = texture(normal_texture, normal_tex_coords_2).xyz * 2.0 - 2.0;
    vec3 tangent_normal_2 = vec3(
      tangent_normal_2_rgb.b, tangent_normal_2_rgb.g, tangent_normal_2_rgb.r
    );
    vec3 tangent_normal = normalize(
      (sin(t) * 0.5 + 0.5) * tangent_normal_1 +
      (cos(t) * 0.5 + 0.5) * tangent_normal_2 +
      unit_normal * 3
    );
    mat3 TBN = mat3(fs_in.tangent, fs_in.bitangent, unit_normal);
    N = normalize(TBN * tangent_normal);
#else
    vec3 normal_from_map_1 = get_normal_from_map(
      texture(normal_texture, normal_tex_coords_1),
      fs_in.world_position,
      unit_normal,
      normal_tex_coords_1
    );
    vec3 normal_from_map_2 = get_normal_from_map(
      texture(normal_texture, normal_tex_coords_2),
      fs_in.world_position,
      unit_normal,
      normal_tex_coords_2
    );
    N = normalize(
      (sin(t) * 0.5 + 0.5) * normal_from_map_1 +
      (cos(t) * 0.5 + 0.5) * normal_from_map_2 +
      unit_normal * 3
    );
#endif
  } else {
    N = unit_normal;
  }

  vec3 V = normalize(camera_position - fs_in.world_position);
  vec3 L = -SUN_DIRECTION;
  vec3 H = normalize(V + L);
  vec3 R = reflect(-L, N);
  float h_dot_v = max(dot(H, V), M_EPSILON);
  float n_dot_v = max(dot(N, V), M_EPSILON);
  float n_dot_l = max(dot(N, L), M_EPSILON);
  float r_dot_l = max(dot(R, L), M_EPSILON);
  float r_dot_v = max(dot(R, V), M_EPSILON);
  vec3 n_min_v = N - V;
  float water_dist = length(camera_position - fs_in.world_position);

  // TODO: Figure out what's up here.
  float F = fresnel_schlick(h_dot_v, WATER_F0);
  // float F = 1.0 + 5.0 * (1.0-smoothstep(0.0,0.2, n_dot_v));
  // float F = pow(clamp(1.0 - dot(N, -V), 0.0, 1.0), 3.0) * 0.5;

  // NOTE: We sample our coordinates from the g_albedo_texture, with an offset
  // to account for refraction. However, because we're not taking depth
  // information in to account, this offset can lead us to sampling
  // an object that is _in front_ of our waves. Hence, we check what's farther,
  // the water, or the sampled "underwater" object. We allow for the object
  // to be within a distance of 1 closer to the camera than the water,
  // otherwise we discard the sampled albedo by setting it to 0.
  vec2 screen_position = gl_FragCoord.xy / vec2(window_width, window_height);
  vec3 underwater_position = texture(g_position_texture, screen_position).rgb;
  vec3 underwater_albedo = vec3(0.0);
  if (underwater_position != vec3(0.0, 0.0, 0.0)) {
    vec2 refraction_dir = normalize(vec2(n_min_v.z, n_min_v.x));
    vec2 refracted_tex_coords = screen_position - refraction_dir * 0.010;
    vec3 refracted_underwater_position = texture(g_position_texture, refracted_tex_coords).rgb;
    float underwater_dist = length(camera_position - refracted_underwater_position);
    float sampled_albedo_discard_factor = max(0, 1 - ((water_dist - underwater_dist) / 1));
    float underwater_distance_factor = (1 - to_unit_interval(
      length(fs_in.world_position - underwater_position),
      0.0,
      WATER_MAX_PASSTHROUGH_DISTANCE
    )) / 2;
    underwater_albedo = texture(g_albedo_texture, refracted_tex_coords).rgb *
      sampled_albedo_discard_factor *
      underwater_distance_factor;
  }

  vec3 foam_color = vec3(0.0);
  if (underwater_position != vec3(0.0, 0.0, 0.0)) {
    // TODO: Clean up these texture lookups maybe.
    vec2 foam_tex_coords = corrected_tex_coords * 90.0;
    float channelA = texture(
      foam_texture, foam_tex_coords - vec2(3.0, cos(corrected_tex_coords.x))
    ).r;
    float channelB = texture(
      foam_texture, foam_tex_coords * 0.5 + vec2(sin(corrected_tex_coords.y), 3.0)
    ).b;
    float mask = clamp(pow((channelA + channelB), 2), 0, 1);
    float water_to_underwater_dist = length(fs_in.world_position - underwater_position);
    float foam_falloff = 1 - (water_to_underwater_dist / WATER_FOAM_DIST);
    float foam_skirt_falloff = 1 - (water_to_underwater_dist / WATER_FOAM_SKIRT_DIST);
    vec3 foam = WATER_FOAM_COLOR * WATER_FOAM_ALPHA * foam_falloff;
    vec3 foam_skirt = WATER_FOAM_COLOR * WATER_FOAM_ALPHA * foam_skirt_falloff;
    foam_color = clamp(
      max(foam - mask, foam_skirt), 0.0, 1.0
    );
  }

  float depth_factor = to_unit_interval(
    fs_in.world_position.y, WATER_MIN_HEIGHT, WATER_MAX_HEIGHT
  );
  vec3 shallow_color = mix(WATER_ALBEDO_DEEP, WATER_ALBEDO_VERY_SHALLOW, depth_factor);

  vec3 reflected =
    SKY_REFLECTION_ALBEDO / 3.0 +
    diffuse(n_dot_l) * SKY_REFLECTION_ALBEDO * 5.0 * SUNSET_LIGHT_FACTOR +
    vec3(0.0);

  vec3 refracted =
    underwater_albedo +
    WATER_ALBEDO_DEEP * SUNSET_LIGHT_FACTOR +
    vec3(0.0);

  vec3 color =
    mix(refracted, reflected, F) +
    specular(r_dot_v) * SUN_ALBEDO * (1.0 / pow(water_dist, 1.3)) +
    shallow_color * SUNSET_LIGHT_FACTOR +
    foam_color +
    vec3(0.0);

  // NOTE: The first directional light is the sun. This is our main
  // light source.
  float shadow = calculate_directional_shadows(
    fs_in.world_position,
    N,
    directional_light_position[0],
    directional_light_direction[0],
    0
  );
  // Take out the very lightest shadows a little bit.
  shadow = pow(shadow, 1.5);
  vec3 Lo = color * (1.0 - (shadow * WATER_SHADOW_FACTOR));
  float non_sunlight_light_factor = 1.00;

  // NOTE: The rest of the directional lights, as well as the point
  // lights, should slightly brighten the water.
  {
    vec3 albedo = color;
    float metallic = 0.0;
    float roughness = 1.0;

    for (int idx_light = 1; idx_light < n_directional_lights; idx_light++) {
      float shadow = calculate_directional_shadows(
        fs_in.world_position,
        N,
        directional_light_position[idx_light],
        directional_light_direction[idx_light],
        idx_light
      );

      Lo += compute_directional_light(
        fs_in.world_position,
        vec3(directional_light_color[idx_light]),
        directional_light_direction[idx_light],
        albedo, metallic, roughness,
        N, V,
        n_dot_v,
        vec3(WATER_F0)
      ) * (1.0 - shadow) * non_sunlight_light_factor;
    }

    for (int idx_light = 0; idx_light < n_point_lights; idx_light++) {
      float shadow = calculate_point_shadows(
        fs_in.world_position,
        N,
        point_light_position[idx_light],
        idx_light
      );

      Lo += compute_point_light(
        fs_in.world_position,
        vec3(point_light_position[idx_light]),
        vec3(point_light_color[idx_light]),
        point_light_attenuation[idx_light],
        albedo, metallic, roughness,
        N, V,
        n_dot_v,
        vec3(WATER_F0)
      ) * (1.0 - shadow) * non_sunlight_light_factor;
    }
  }

  l_color = vec4(Lo, 1.0);
  float brightness = dot(l_color.rgb, BLOOM_BRIGHTNESS_THRESHOLD);
  if (brightness > 1.0) {
    l_bright_color = l_color;
  } else {
    l_bright_color = vec4(0.0, 0.0, 0.0, 1.0);
  }
}

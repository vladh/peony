uniform mat4 model;
uniform mat4 mesh_transform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out BLOCK {
  vec3 world_position;
  vec2 screen_position;
  vec3 normal;
  vec3 bitangent;
  vec3 tangent;
  vec2 tex_coords;
} vs_out;


vec2 random2(vec2 st){
  st = vec2( dot(st,vec2(127.1,311.7)),
    dot(st,vec2(269.5,183.3)) );
  return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
float noise(vec2 st) {
  vec2 i = floor(st);
  vec2 f = fract(st);

  vec2 u = f*f*(3.0-2.0*f);

  return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
      dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
    mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
      dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}


float get_nice_noise(vec2 st) {
  return noise(st + vec2(t / 2)) / 2;
  /* return noise(st); */
}


void main() {
  vec3 prelim_world_position = vec3(model * mesh_transform * vec4(position, 1.0));

  /* vec3 water_position = water_make_position(prelim_world_position.xz); */
  /* vec3 water_normal; */
  /* vec3 water_bitangent; */
  /* vec3 water_tangent; */
  /* water_make_normal(water_position, water_normal, water_bitangent, water_tangent); */
  const float delta = 0.01;

  vec3 water_position = vec3(
    prelim_world_position.x,
    get_nice_noise(vec2(prelim_world_position.x, prelim_world_position.z)),
    prelim_world_position.z
  );

  vec3 water_position_dx = vec3(
    prelim_world_position.x + delta,
    get_nice_noise(vec2(prelim_world_position.x + delta, prelim_world_position.z)),
    prelim_world_position.z
  );
  vec3 water_tangent = normalize(water_position_dx - water_position);

  vec3 water_position_dz = vec3(
    prelim_world_position.x,
    get_nice_noise(vec2(prelim_world_position.x, prelim_world_position.z + delta)),
    prelim_world_position.z + delta
  );
  vec3 water_bitangent = normalize(water_position_dz - water_position);

  vec3 water_normal = normalize(cross(water_bitangent, water_tangent));

  gl_Position = projection * view * vec4(water_position, 1.0);

  vec3 ndc = gl_Position.xyz / gl_Position.w;
  vs_out.screen_position = ndc.xy * 0.5 + 0.5;

  vs_out.world_position = water_position;
  vs_out.normal = water_normal;
  vs_out.bitangent = water_bitangent;
  vs_out.tangent = water_tangent;
  vs_out.tex_coords = tex_coords;
}

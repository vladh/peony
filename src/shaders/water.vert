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

float hash(vec3 p)  // replace this by something better
{
  p  = 50.0*fract( p*0.3183099 + vec3(0.71,0.113,0.419));
  return -1.0+2.0*fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}


// return value noise (in x) and its derivatives (in yzw)
vec4 noised( in vec3 x )
{
  vec3 i = floor(x);
  vec3 w = fract(x);

#if 1
  // quintic interpolation
  vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
  vec3 du = 30.0*w*w*(w*(w-2.0)+1.0);
#else
  // cubic interpolation
  vec3 u = w*w*(3.0-2.0*w);
  vec3 du = 6.0*w*(1.0-w);
#endif

  float a = hash(i+vec3(0.0,0.0,0.0));
  float b = hash(i+vec3(1.0,0.0,0.0));
  float c = hash(i+vec3(0.0,1.0,0.0));
  float d = hash(i+vec3(1.0,1.0,0.0));
  float e = hash(i+vec3(0.0,0.0,1.0));
  float f = hash(i+vec3(1.0,0.0,1.0));
  float g = hash(i+vec3(0.0,1.0,1.0));
  float h = hash(i+vec3(1.0,1.0,1.0));

  float k0 =   a;
  float k1 =   b - a;
  float k2 =   c - a;
  float k3 =   e - a;
  float k4 =   a - b - c + d;
  float k5 =   a - c - e + g;
  float k6 =   a - b - e + f;
  float k7 = - a + b + c - d + e - f - g + h;

  return vec4( k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z,
    du * vec3( k1 + k4*u.y + k6*u.z + k7*u.y*u.z,
      k2 + k5*u.z + k4*u.x + k7*u.z*u.x,
      k3 + k6*u.x + k5*u.y + k7*u.x*u.y ) );
}



float get_nice_noise(vec2 st) {
  /* return noise(st + vec2(t / 2)) / 2; */
  /* return noise(st / 2.0); */
  return noise(st);
  /* return sin((st.x + st.y) / 2.0); */
}


void main() {
  vec3 prelim_world_position = vec3(model * mesh_transform * vec4(position, 1.0));

  /* vec3 water_position = water_make_position(prelim_world_position.xz); */
  /* vec3 water_normal; */
  /* vec3 water_bitangent; */
  /* vec3 water_tangent; */
  /* water_make_normal(water_position, water_normal, water_bitangent, water_tangent); */
  const float delta = 0.1;

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
  vec3 water_bitangent = normalize(water_position_dx - water_position);

  vec3 water_position_dz = vec3(
    prelim_world_position.x,
    get_nice_noise(vec2(prelim_world_position.x, prelim_world_position.z + delta)),
    prelim_world_position.z + delta
  );
  vec3 water_tangent = normalize(water_position_dz - water_position);

  /* float y_dx = get_nice_noise(vec2(prelim_world_position.x + delta, prelim_world_position.z)); */
  /* float dy_dx = y_dx - water_position.y; */
  /* vec3 water_bitangent = normalize(vec3(delta, dy_dx, 0.0)); */

  /* float y_dz = get_nice_noise(vec2(prelim_world_position.x, prelim_world_position.z + delta)); */
  /* float dy_dz = y_dz - water_position.y; */
  /* vec3 water_tangent = normalize(vec3(0.0, dy_dz, delta)); */

  /* vec4 noise_terms = noised(prelim_world_position); */
  /* vec3 water_position = vec3( */
  /*   prelim_world_position.x, */
  /*   noise_terms[0], */
  /*   prelim_world_position.z */
  /* ); */
  /* vec3 water_bitangent = normalize(vec3(1.0, noise_terms[1], 0.0)); */
  /* vec3 water_tangent = normalize(vec3(0.0, noise_terms[3], 1.0)); */

  vec3 water_normal = normalize(cross(water_tangent, water_bitangent));

  /* vec3 water_normal = vec3(sin(prelim_world_position.x), 0.0, sin(prelim_world_position.z)); */

  /* vec3 off = vec3(1.0, 1.0, 0.0) / 1; */
  /* float hL = get_nice_noise(prelim_world_position.xz - off.xz); */
  /* float hR = get_nice_noise(prelim_world_position.xz + off.xz); */
  /* float hD = get_nice_noise(prelim_world_position.xz - off.zy); */
  /* float hU = get_nice_noise(prelim_world_position.xz + off.zy); */
  /* vec3 water_bitangent = vec3(1.0, 0.0, 0.0); */
  /* vec3 water_tangent = vec3(0.0, 0.0, 1.0); */
  /* vec3 water_normal = normalize(vec3(hL - hR, 2.0, hD - hU)); */

  gl_Position = projection * view * vec4(water_position, 1.0);

  vec3 ndc = gl_Position.xyz / gl_Position.w;
  vs_out.screen_position = ndc.xy * 0.5 + 0.5;

  vs_out.world_position = water_position;
  vs_out.normal = water_normal;
  /* vs_out.world_position = vec3(model * mesh_transform * vec4(water_position, 1.0)); */
  /* vs_out.normal = normalize(mat3(transpose(inverse(model * mesh_transform))) * water_normal); */
  vs_out.bitangent = water_bitangent;
  vs_out.tangent = water_tangent;
  vs_out.tex_coords = tex_coords;
}

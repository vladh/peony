uniform sampler2D source_texture;

in BLOCK {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;


void main() {
  vec3 color = vec3(0.0);

  vec2 direction;
  if (is_blur_horizontal) {
    direction = vec2(1.0, 0.0);
  } else {
    direction = vec2(0.0, 1.0);
  }
  vec4 sum = vec4(0.0);
  vec2 tc = fs_in.tex_coords;
  vec2 texelSize = 1.0 / textureSize(source_texture, 0);
  vec2 blur = texelSize * direction;

  sum += texture2D(source_texture, vec2(tc.x, tc.y)) * BLUR_WEIGHTS[0];

  for (int idx = 1; idx < 3; idx++) {
    float offset = BLUR_OFFSETS[idx];
    // float offset = idx;
    sum += texture2D(
      source_texture, tc - vec2(blur.x * offset, blur.y * offset)
    ) * BLUR_WEIGHTS[idx];
    sum += texture2D(
      source_texture, tc + vec2(blur.x * offset, blur.y * offset)
    ) * BLUR_WEIGHTS[idx];
  }

  color = sum.rgb;
  frag_color = vec4(color, 1.0);
}

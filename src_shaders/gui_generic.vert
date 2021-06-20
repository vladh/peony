/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec4 color;

out BLOCK {
  vec2 tex_coords;
  vec4 color;
} vs_out;

void main() {
  gl_Position = ui_projection * vec4(position, 0.0, 1.0);
  vs_out.tex_coords = tex_coords;
  vs_out.color = color;
}

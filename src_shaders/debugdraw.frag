/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

in BLOCK {
  vec4 color;
} fs_in;

out vec4 frag_color;

void main() {
  frag_color = fs_in.color;
}

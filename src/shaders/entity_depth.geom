#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadow_transforms[6];

out GS_OUT {
  vec4 frag_position;
} gs_out;

void main() {
  for(int face = 0; face < 6; face++) {
    // built-in variable that specifies to which face we render.
    gl_Layer = face;

    // for each triangle's vertices
    for(int i = 0; i < 3; ++i) {
      gs_out.frag_position = gl_in[i].gl_Position;
      gl_Position = shadowMatrices[face] * gs_out.frag_position;
      EmitVertex();
    }

    EndPrimitive();
  }
}

out vec4 frag_color;

in BLOCK {
  vec3 normal;
} fs_in;

// NOTE: Normally, we might have different kind of outputs.
//
// Deferred rendering
// (vec3 g_position, vec3 g_normal, vec4 g_albedo, vec4 g_pbr).
//
// Forward rendering
// (vec4 frag_color)
//
// Forward rendering with HDR
// (vec4 l_color, vec4 l_bright_color)
//
// We're not gonna care about any of that!
// Just setting our one output to pink will set EVERYTHING pink.

void main() {
  frag_color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
}

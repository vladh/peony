#ifndef TEXTURE_ATLAS_HPP
#define TEXTURE_ATLAS_HPP

class TextureAtlas {
public:
  glm::ivec2 size;
  glm::ivec2 next_position;
  glm::ivec2 max_allocated_position_per_axis;
  uint32 texture_name;

  TextureAtlas(glm::ivec2 size);
  glm::ivec2 push_space(glm::ivec2 space_size);
};

#endif

TextureAtlas::TextureAtlas(
  glm::ivec2 size
) :
  size(size)
{
  this->next_position = glm::ivec2(0, 0);
  this->max_allocated_position_per_axis = glm::ivec2(0, 0);

  glGenTextures(1, &this->texture_name);
  glBindTexture(GL_TEXTURE_2D, this->texture_name);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RED,
    this->size.x, this->size.y,
    0, GL_RED, GL_UNSIGNED_BYTE, 0
  );
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

glm::ivec2 TextureAtlas::push_space(glm::ivec2 space_size) {
  // New space in a texture is first allocated along the x-axis.
  // If we run over the end of the x-axis, we go to the next "row" from the
  // beginning of the x-axis.
  glm::ivec2 new_space_position = this->next_position;
  glm::ivec2 new_space_end = this->next_position + space_size;

  // If we run past the end of the y-axis, we've filled up the texture.
  // This is a problem. We'll start reallocating from the beginning,
  // overriding old stuff.
  if (new_space_end.y > this->size.y) {
    log_error("Ran past y-axis end of TextureAtlas.");
    // Maybe we just start overwriting stuff here.
    new_space_position = glm::ivec2(0, 0);
  }

  // If we run past the end of the x-axis, move on to the next row.
  if (new_space_end.x > this->size.x) {
    new_space_position = glm::ivec2(0, this->max_allocated_position_per_axis.y);
  }

  this->max_allocated_position_per_axis = glm::max(
    this->max_allocated_position_per_axis, new_space_end
  );
  this->next_position = new_space_position + glm::ivec2(space_size.x, 0);
  return new_space_position;
}

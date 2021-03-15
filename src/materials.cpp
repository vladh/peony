Texture* Materials::init_texture(
  Texture *texture,
  TextureType type,
  const char* path
) {
  texture->type = type;
  strcpy(texture->path, TEXTURE_DIR);
  strcat(texture->path, path);
  texture->target = GL_TEXTURE_2D;
  texture->is_screensize_dependent = is_texture_type_screensize_dependent(type);
  return texture;
}


Texture* Materials::init_texture(
  Texture *texture,
  GLenum target,
  TextureType type,
  uint32 texture_name,
  int32 width,
  int32 height,
  int32 n_components
) {
  texture->target = target;
  texture->type = type;
  texture->texture_name = texture_name;
  texture->width = width;
  texture->height = height;
  texture->n_components = n_components;
  texture->is_screensize_dependent = is_texture_type_screensize_dependent(type);
  return texture;
}


bool32 Materials::is_texture_type_screensize_dependent(TextureType type) {
  return (
    type == TextureType::g_position ||
    type == TextureType::g_normal ||
    type == TextureType::g_albedo ||
    type == TextureType::g_pbr ||
    type == TextureType::l_color ||
    type == TextureType::l_bright_color ||
    type == TextureType::l_depth ||
    type == TextureType::blur1 ||
    type == TextureType::blur2
  );
}


const char* Materials::texture_type_to_string(TextureType texture_type) {
  if (texture_type == TextureType::none) {
    return "none";
  } else if (texture_type == TextureType::albedo) {
    return "albedo";
  } else if (texture_type == TextureType::metallic) {
    return "metallic";
  } else if (texture_type == TextureType::roughness) {
    return "roughness";
  } else if (texture_type == TextureType::ao) {
    return "ao";
  } else if (texture_type == TextureType::normal) {
    return "normal";
  } else if (texture_type == TextureType::shadowmap) {
    return "shadowmap";
  } else if (texture_type == TextureType::other) {
    return "other";
  } else if (texture_type == TextureType::g_position) {
    return "g_position";
  } else if (texture_type == TextureType::g_normal) {
    return "g_normal";
  } else if (texture_type == TextureType::g_albedo) {
    return "g_albedo";
  } else if (texture_type == TextureType::g_pbr) {
    return "g_pbr";
  } else if (texture_type == TextureType::l_color) {
    return "l_color";
  } else if (texture_type == TextureType::l_bright_color) {
    return "l_bright_color";
  } else if (texture_type == TextureType::l_depth) {
    return "l_depth";
  } else if (texture_type == TextureType::blur1) {
    return "blur1";
  } else if (texture_type == TextureType::blur2) {
    return "blur2";
  } else {
    log_error("Could not convert TextureType to string: %d", texture_type);
    return "<unknown>";
  }
}


TextureType Materials::texture_type_from_string(const char* str) {
  if (strcmp(str, "none") == 0) {
    return TextureType::none;
  } else if (strcmp(str, "albedo") == 0) {
    return TextureType::albedo;
  } else if (strcmp(str, "metallic") == 0) {
    return TextureType::metallic;
  } else if (strcmp(str, "roughness") == 0) {
    return TextureType::roughness;
  } else if (strcmp(str, "ao") == 0) {
    return TextureType::ao;
  } else if (strcmp(str, "normal") == 0) {
    return TextureType::normal;
  } else if (strcmp(str, "shadowmap") == 0) {
    return TextureType::shadowmap;
  } else if (strcmp(str, "other") == 0) {
    return TextureType::other;
  } else if (strcmp(str, "g_position") == 0) {
    return TextureType::g_position;
  } else if (strcmp(str, "g_normal") == 0) {
    return TextureType::g_normal;
  } else if (strcmp(str, "g_albedo") == 0) {
    return TextureType::g_albedo;
  } else if (strcmp(str, "g_pbr") == 0) {
    return TextureType::g_pbr;
  } else if (strcmp(str, "l_color") == 0) {
    return TextureType::l_color;
  } else if (strcmp(str, "l_bright_color") == 0) {
    return TextureType::l_bright_color;
  } else if (strcmp(str, "l_depth") == 0) {
    return TextureType::l_depth;
  } else if (strcmp(str, "blur1") == 0) {
    return TextureType::blur1;
  } else if (strcmp(str, "blur2") == 0) {
    return TextureType::blur2;
  } else {
    log_fatal("Could not parse TextureType from string: %s", str);
    return TextureType::none;
  }
}


TextureAtlas* Materials::init_texture_atlas(
  TextureAtlas* atlas,
  glm::ivec2 size
) {
  atlas->size = size;
  atlas->next_position = glm::ivec2(0, 0);
  atlas->max_allocated_position_per_axis = glm::ivec2(0, 0);

  glGenTextures(1, &atlas->texture_name);
  glBindTexture(GL_TEXTURE_2D, atlas->texture_name);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RED,
    atlas->size.x, atlas->size.y,
    0, GL_RED, GL_UNSIGNED_BYTE, 0
  );
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return atlas;
}

glm::ivec2 Materials::push_space_to_texture_atlas(
  TextureAtlas* atlas,
  glm::ivec2 space_size
) {
  // New space in a texture is first allocated along the x-axis.
  // If we run over the end of the x-axis, we go to the next "row" from the
  // beginning of the x-axis.
  glm::ivec2 new_space_position = atlas->next_position;
  glm::ivec2 new_space_end = atlas->next_position + space_size;

  // If we run past the end of the y-axis, we've filled up the texture.
  // This is a problem. We'll start reallocating from the beginning,
  // overriding old stuff.
  if (new_space_end.y > atlas->size.y) {
    log_error("Ran past y-axis end of TextureAtlas.");
    // Maybe we just start overwriting stuff here.
    new_space_position = glm::ivec2(0, 0);
  }

  // If we run past the end of the x-axis, move on to the next row.
  if (new_space_end.x > atlas->size.x) {
    new_space_position = glm::ivec2(0, atlas->max_allocated_position_per_axis.y);
  }

  atlas->max_allocated_position_per_axis = glm::max(
    atlas->max_allocated_position_per_axis, new_space_end
  );
  atlas->next_position = new_space_position + glm::ivec2(space_size.x, 0);
  return new_space_position;
}


Material* Materials::init_material(
  Material *material,
  MemoryPool *memory_pool
) {
  material->textures =
    Array<Texture>(memory_pool, 16, "textures");

  return material;
};


void Materials::add_texture_to_material(
  Material *material, Texture texture, const char *uniform_name
) {
  if (texture.is_screensize_dependent) {
    material->is_screensize_dependent = true;
  }
  material->textures.push(texture);
  strcpy(
    material->texture_uniform_names[material->idx_texture_uniform_names++],
    uniform_name
  );
}


void Materials::copy_material_textures_to_pbo(
  Material *material,
  PersistentPbo *persistent_pbo
) {
  for (uint32 idx = 0; idx < material->textures.size; idx++) {
    Texture *texture = material->textures[idx];
    if (texture->texture_name) {
      continue;
    }
    unsigned char *image_data = Util::load_image(
      texture->path, &texture->width, &texture->height,
      &texture->n_components, true
    );
    texture->pbo_idx_for_copy = get_new_persistent_pbo_idx(persistent_pbo);
    memcpy(
      get_memory_for_persistent_pbo_idx(persistent_pbo, texture->pbo_idx_for_copy),
      image_data,
      texture->width * texture->height * texture->n_components
    );
    Util::free_image(image_data);
  }
}


void Materials::generate_textures_from_pbo(
  Material *material,
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool
) {
  if (material->have_textures_been_generated) {
    log_warning("Tried to generate textures but they've already been generated.");
  }
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, persistent_pbo->pbo);

  for (uint32 idx = 0; idx < material->textures.size; idx++) {
    Texture *texture = material->textures[idx];
    if (texture->texture_name != 0) {
      continue;
    }
    texture->texture_name = Materials::get_new_texture_name(
      texture_name_pool, texture->width
    );
    glBindTexture(GL_TEXTURE_2D, texture->texture_name);
    glTexSubImage2D(
      GL_TEXTURE_2D, 0, 0, 0,
      texture->width, texture->height,
      Util::get_texture_format_from_n_components(texture->n_components),
      GL_UNSIGNED_BYTE,
      get_offset_for_persistent_pbo_idx(persistent_pbo, texture->pbo_idx_for_copy)
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    if (texture->type == TextureType::normal) {
      material->should_use_normal_map = true;
    }
  }

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  material->have_textures_been_generated = true;
}


PersistentPbo* Materials::init_persistent_pbo(
  PersistentPbo *ppbo,
  uint16 texture_count, int32 width, int32 height, int32 n_components
) {
  ppbo->texture_count = texture_count;
  ppbo->width = width;
  ppbo->height = height;
  ppbo->n_components = n_components;
  ppbo->texture_size = width * height * n_components;
  ppbo->total_size = ppbo->texture_size * ppbo->texture_count;

  glGenBuffers(1, &ppbo->pbo);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, ppbo->pbo);

  GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  glBufferStorage(GL_PIXEL_UNPACK_BUFFER, ppbo->total_size, 0, flags);
  ppbo->memory = glMapBufferRange(
    GL_PIXEL_UNPACK_BUFFER, 0, ppbo->total_size, flags
  );

  // We need to unbind this or it will mess up some textures transfers
  // after this function.
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  return ppbo;
}

void Materials::delete_persistent_pbo(PersistentPbo *ppbo) {
  glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
  glDeleteBuffers(1, &ppbo->pbo);
}


uint16 Materials::get_new_persistent_pbo_idx(PersistentPbo *ppbo) {
  uint16 current_idx = ppbo->next_idx;
  ppbo->next_idx++;
  if (ppbo->next_idx >= ppbo->texture_count) {
    ppbo->next_idx = 0;
  }
  return current_idx;
}


void* Materials::get_offset_for_persistent_pbo_idx(
  PersistentPbo *ppbo, uint16 idx
) {
  return (void*)((uint64)idx * ppbo->texture_size);
}


void* Materials::get_memory_for_persistent_pbo_idx(
  PersistentPbo *ppbo, uint16 idx
) {
  return (char*)ppbo->memory + ((uint64)idx * ppbo->texture_size);
}


TextureNamePool* Materials::init_texture_name_pool(
  TextureNamePool *pool,
  MemoryPool *memory_pool,
  uint32 n_textures,
  uint32 mipmap_max_level
) {
  pool->n_textures = n_textures;
  pool->mipmap_max_level = mipmap_max_level;
  pool->n_sizes = 0;
  pool->sizes[pool->n_sizes++] = 256;
  pool->sizes[pool->n_sizes++] = 512;
  pool->sizes[pool->n_sizes++] = 2048;

  pool->texture_names = (uint32*)Memory::push(
    memory_pool,
    sizeof(uint32) * pool->n_textures * pool->n_sizes,
    "texture_names"
  );

  glGenTextures(
    pool->n_textures * pool->n_sizes,
    pool->texture_names
  );

  for (uint32 idx_size = 0; idx_size < pool->n_sizes; idx_size++) {
    for (uint32 idx_for_size = 0; idx_for_size < pool->n_textures; idx_for_size++) {
      uint32 idx_name = (idx_size * pool->n_textures) + idx_for_size;
      glBindTexture(GL_TEXTURE_2D, pool->texture_names[idx_name]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, pool->mipmap_max_level);
      glTexStorage2D(
        GL_TEXTURE_2D, pool->mipmap_max_level + 1, GL_RGBA8,
        pool->sizes[idx_size], pool->sizes[idx_size]
      );
    }
  }

  return pool;
}


uint32 Materials::get_new_texture_name(
  TextureNamePool *pool,
  uint32 target_size
) {
  for (uint32 idx_size = 0; idx_size < pool->n_sizes; idx_size++) {
    if (pool->sizes[idx_size] == target_size) {
      assert(pool->idx_next[idx_size] < pool->n_textures);
      uint32 idx_name = (idx_size * pool->n_textures) + pool->idx_next[idx_size];
      uint32 texture_name = pool->texture_names[idx_name];
      pool->idx_next[idx_size]++;
      return texture_name;
    }
  }
  log_fatal(
    "Could not make texture of size %d as there is no pool for that size.",
    target_size
  );
  return 0;
}

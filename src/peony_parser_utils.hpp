#pragma once

#include "array.hpp"
#include "peony_parser.hpp"
#include "models.hpp"
#include "renderer.hpp"

namespace peony_parser_utils {
  constexpr const char *TEXTURE_PREFIX = "textures.";
  constexpr size_t TEXTURE_PREFIX_LENGTH = 9;
  constexpr const char *BUILTIN_TEXTURE_PREFIX = "builtin_textures.";
  constexpr size_t BUILTIN_TEXTURE_PREFIX_LENGTH = 17;

  char* get_string(PeonyFileProp *prop);
  bool32* get_boolean(PeonyFileProp *prop);
  real32* get_number(PeonyFileProp *prop);
  v2* get_vec2(PeonyFileProp *prop);
  v3* get_vec3(PeonyFileProp *prop);
  v4* get_vec4(PeonyFileProp *prop);
  PeonyFileProp* find_prop(PeonyFileEntry *entry, char const *name);
  void get_unique_string_values_for_prop_name(
    PeonyFile *pf,
    Array<char[MAX_COMMON_NAME_LENGTH]> *unique_values,
    char const *prop_name
  );
  void create_material_from_peony_file_entry(
    Material *material,
    PeonyFileEntry *entry,
    BuiltinTextures *builtin_textures,
    MemoryPool *memory_pool
  );
  void create_model_loader_from_peony_file_entry(
    PeonyFileEntry *entry,
    EntityHandle entity_handle,
    ModelLoader *model_loader
  );
  void create_entity_loader_from_peony_file_entry(
    PeonyFileEntry *entry,
    EntityHandle entity_handle,
    EntityLoader *entity_loader
  );
}

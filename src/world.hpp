#ifndef WORLD_HPP
#define WORLD_HPP

namespace World {
  void get_entity_text_representation(
    char *text, State *state, Entity *entity, uint8 depth
  );
  void get_scene_text_representation(char *text, State *state);
  void update_light_position(State *state, real32 amount);
  void create_entities_from_entity_template(
    EntityTemplate *entity_template,
    MemoryPool *memory_pool,
    EntitySet *entity_set,
    Array<ModelAsset> *model_assets,
    Array<ShaderAsset> *shader_assets,
    State *state
  );
  void create_internal_entities(MemoryPool *memory_pool, State *state);
  void init(
    MemoryPool *asset_memory_pool,
    MemoryPool *entity_memory_pool,
    State *state
  );
  void check_all_model_assets_loaded(State *state);
  void update(State *state);
};

#endif

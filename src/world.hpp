#ifndef WORLD_HPP
#define WORLD_HPP

namespace World {
  void get_entity_text_representation(
    char *text, State *state, Entities::Entity *entity, uint8 depth
  );
  void get_scene_text_representation(char *text, State *state);
  void update_light_position(State *state, real32 amount);
  void create_entities_from_entity_template(
    PeonyFileParser::EntityTemplate *entity_template,
    Memory *memory,
    EntitySets::EntitySet *entity_set,
    Array<Models::ModelAsset> *model_assets,
    Array<Shaders::ShaderAsset> *shader_assets,
    State *state
  );
  void create_internal_entities(Memory *memory, State *state);
  void init(Memory *memory, State *state);
  void check_all_model_assets_loaded(Memory *memory, State *state);
  void update(Memory *memory, State *state);
};

#endif

#ifndef WORLD_HPP
#define WORLD_HPP

namespace World {
  void get_entity_text_representation(
    char *text, State *state, Entity *entity, uint8 depth
  );
  void get_scene_text_representation(char *text, State *state);
  void get_materials_text_representation(char *text, State *state);
  void update_light_position(State *state, real32 amount);
  void create_entity_loader_from_entity_template(
    EntityTemplate *entity_template,
    EntitySet *entity_set,
    EntityLoaderSet *entity_loader_set,
    State *state
  );
  void create_internal_materials(State *state);
  void create_internal_entities(State *state);
  void destroy_non_internal_materials(State *state);
  void destroy_non_internal_entities(State *state);
  void load_scene(const char *scene_path, State *state);
  void init(State *state);
  bool32 check_all_entities_loaded(State *state);
  void update(State *state);
};

#endif

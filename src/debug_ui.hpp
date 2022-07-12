// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "engine.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include "mats.hpp"
#include "input.hpp"

class debug_ui {
public:
    static void render_debug_ui(WindowSize *window_size);

private:
    static void get_entity_text_representation(char *text, entities::Entity *entity, u8 depth);
    static void get_scene_text_representation(char *text);
    static void get_materials_text_representation(char *text);
};

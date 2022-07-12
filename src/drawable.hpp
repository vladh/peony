// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "geom.hpp"

class drawable {
public:
    enum class Mode { regular, depth };

    enum class Pass : u32 {
        none = 0,
        shadowcaster = (1 << 0),
        deferred = (1 << 1),
        forward_depth = (1 << 2),
        forward_nodepth = (1 << 3),
        forward_skybox = (1 << 4),
        lighting = (1 << 5),
        postprocessing = (1 << 6),
        preblur = (1 << 7),
        blur1 = (1 << 8),
        blur2 = (1 << 9),
        renderdebug = (1 << 10),
    };

    struct Component {
        entities::Handle entity_handle;
        geom::Mesh mesh;
        Pass target_render_pass = Pass::none;
    };

    struct ComponentSet {
        Array<drawable::Component> components;
        u32 last_drawn_shader_program;
    };

    static char const * render_pass_to_string(drawable::Pass render_pass);
    static drawable::Pass render_pass_from_string(const char* str);
    static bool is_component_valid(drawable::Component *drawable_component);
    static void destroy_component(drawable::Component *drawable_component);
};

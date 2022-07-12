// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "debug.hpp"
#include "util.hpp"
#include "logs.hpp"
#include "models.hpp"


char const *
drawable::render_pass_to_string(drawable::Pass render_pass)
{
    if (render_pass == drawable::Pass::none) {
        return "none";
    } else if (render_pass == drawable::Pass::shadowcaster) {
        return "shadowcaster";
    } else if (render_pass == drawable::Pass::deferred) {
        return "deferred";
    } else if (render_pass == drawable::Pass::forward_depth) {
        return "forward_depth";
    } else if (render_pass == drawable::Pass::forward_nodepth) {
        return "forward_nodepth";
    } else if (render_pass == drawable::Pass::forward_skybox) {
        return "forward_skybox";
    } else if (render_pass == drawable::Pass::lighting) {
        return "lighting";
    } else if (render_pass == drawable::Pass::postprocessing) {
        return "postprocessing";
    } else if (render_pass == drawable::Pass::preblur) {
        return "preblur";
    } else if (render_pass == drawable::Pass::blur1) {
        return "blur1";
    } else if (render_pass == drawable::Pass::blur2) {
        return "blur2";
    } else if (render_pass == drawable::Pass::renderdebug) {
        return "renderdebug";
    } else {
        logs::error("Don't know how to convert drawable::Pass to string: %d", render_pass);
        return "<unknown>";
    }
}


drawable::Pass
drawable::render_pass_from_string(const char* str)
{
    if (pstr_eq(str, "none")) {
        return drawable::Pass::none;
    } else if (pstr_eq(str, "shadowcaster")) {
        return drawable::Pass::shadowcaster;
    } else if (pstr_eq(str, "deferred")) {
        return drawable::Pass::deferred;
    } else if (pstr_eq(str, "forward_depth")) {
        return drawable::Pass::forward_depth;
    } else if (pstr_eq(str, "forward_nodepth")) {
        return drawable::Pass::forward_nodepth;
    } else if (pstr_eq(str, "forward_skybox")) {
        return drawable::Pass::forward_skybox;
    } else if (pstr_eq(str, "lighting")) {
        return drawable::Pass::lighting;
    } else if (pstr_eq(str, "postprocessing")) {
        return drawable::Pass::postprocessing;
    } else if (pstr_eq(str, "preblur")) {
        return drawable::Pass::preblur;
    } else if (pstr_eq(str, "blur1")) {
        return drawable::Pass::blur1;
    } else if (pstr_eq(str, "blur2")) {
        return drawable::Pass::blur2;
    } else if (pstr_eq(str, "renderdebug")) {
        return drawable::Pass::renderdebug;
    } else {
        logs::fatal("Could not parse drawable::Pass: %s", str);
        return drawable::Pass::none;
    }
}


bool
drawable::is_component_valid(drawable::Component *drawable_component)
{
    return geom::is_mesh_valid(&drawable_component->mesh);
}


void
drawable::destroy_component(drawable::Component *drawable_component)
{
    if (!is_component_valid(drawable_component)) {
        return;
    }
    geom::destroy_mesh(&drawable_component->mesh);
}


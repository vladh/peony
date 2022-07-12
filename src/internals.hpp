// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "engine.hpp"
#include "renderer.hpp"
#include "mats.hpp"

class internals {
public:
    static void create_internal_materials();
    static void create_internal_entities();
    static void init();
};

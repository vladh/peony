// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "array.hpp"
#include "mats.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

class fonts {
public:
    // NOTE: Unicode is only the same as ASCII until 0x7F.
    // We can change this to 0xFF when we add Unicode support.
    static constexpr u32 CHAR_MAX_CODEPOINT_TO_LOAD = 0x7F;

    struct Character {
        iv2 size;
        iv2 bearing;
        iv2 advance;
        iv2 tex_coords;
    };

    struct FontAsset {
        const char *name;
        Array<Character> characters;
        u32 texture;
        u32 font_size;
        u32 units_per_em;
        u32 ascender;
        u32 descender;
        u32 height;
    };

    static f32 frac_px_to_px(u32 n);
    static f32 font_unit_to_px(u32 n);
    static FontAsset * get_by_name(Array<FontAsset> *assets, const char *name);
    static FontAsset * init_font_asset(
        FontAsset *font_asset,
        memory::Pool *memory_pool,
        mats::TextureAtlas *texture_atlas,
        FT_Library *ft_library,
        const char *name,
        const char *filename,
        u16 font_size
    );

private:
    static void load_glyphs(
        FontAsset *font_asset,
        FT_Face face,
        mats::TextureAtlas *texture_atlas
    );
};

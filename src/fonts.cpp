// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "../src_external/glad/glad.h"
#include "../src_external/pstr.h"
#include "logs.hpp"
#include "array.hpp"
#include "mats.hpp"
#include "fonts.hpp"
#include "intrinsics.hpp"


real32
fonts::frac_px_to_px(uint32 n)
{
    return (real32)(n >> 6);
}


real32
fonts::font_unit_to_px(uint32 n)
{
    // NOTE: We should be dividing by units_per_em here...probably?
    // This is because we expect height etc. to be in "font units".
    // But treating these metrics as "fractional pixels" seems to work,
    // whereas division by units_per_em doesn't.
    // Check this in more detail.
    return (real32)(n >> 6);
}


fonts::FontAsset *
fonts::get_by_name(Array<FontAsset> *assets, const char *name)
{
    each (asset, *assets) {
        if (pstr_eq(asset->name, name)) {
            return asset;
        }
    }
    logs::warning("Could not find FontAsset with name %s", name);
    return nullptr;
}


fonts::FontAsset *
fonts::init_font_asset(
    FontAsset *font_asset,
    MemoryPool *memory_pool,
    mats::TextureAtlas *texture_atlas,
    FT_Library *ft_library,
    const char *name,
    const char *filename,
    uint16 font_size
) {
    font_asset->name = name;
    font_asset->font_size = font_size;

    char path[MAX_PATH];
    strcpy(path, FONTS_DIR);
    strcat(path, filename);

    font_asset->characters = Array<Character>(memory_pool, CHAR_MAX_CODEPOINT_TO_LOAD + 1, "characters");

    FT_Face face;
    if (FT_New_Face(*ft_library, path, 0, &face)) {
        logs::error("Could not load font at %s", path);
    }
    FT_Set_Pixel_Sizes(face, 0, font_asset->font_size);
    if (!FT_IS_SCALABLE(face)) {
        logs::fatal("Font face not scalable, don't know what to do.");
    }
    font_asset->units_per_em = face->units_per_EM;
    font_asset->ascender = face->ascender;
    font_asset->descender = face->descender;
    font_asset->height = face->height;

    load_glyphs(font_asset, face, texture_atlas);

    FT_Done_Face(face);

    return font_asset;
}


void
fonts::load_glyphs(
    FontAsset *font_asset,
    FT_Face face,
    mats::TextureAtlas *texture_atlas
) {
    FT_GlyphSlot glyph = face->glyph;

    // TODO: Can we avoid loading all characters twice here?
    for (uint32 c = 0; c < CHAR_MAX_CODEPOINT_TO_LOAD; c++) {
        Character *character = font_asset->characters.push();

        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            logs::error("Failed to load glyph %s", c);
            continue;
        }

        character->size = iv2(glyph->bitmap.width, glyph->bitmap.rows);
        character->bearing = iv2(glyph->bitmap_left, glyph->bitmap_top);
        character->advance = iv2(glyph->advance.x, glyph->advance.y);
    }

    mats::activate_font_texture(texture_atlas->texture_name);

    for (uint32 c = 0; c < CHAR_MAX_CODEPOINT_TO_LOAD; c++) {
        if (
            // Unicode C0 controls
            (c <= 0x1F) ||
            // DEL
            (c == 0x7F) ||
            // Unicode C1 controls
            (c >= 0x80 && c <= 0x9F)
        ) {
            continue;
        }

        Character *character = font_asset->characters[c];

        iv2 tex_coords = mats::push_space_to_texture_atlas(texture_atlas, character->size);

        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            logs::error("Failed to load glyph %s", c);
            continue;
        }

        mats::push_font_texture(tex_coords, character->size, glyph->bitmap.buffer);

        character->tex_coords = tex_coords;
    }
}

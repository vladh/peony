// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

static constexpr v3 CUBEMAP_OFFSETS[6] = {
    v3(1.0f, 0.0f, 0.0f),
    v3(-1.0f, 0.0f, 0.0f),
    v3(0.0f, 1.0f, 0.0f),
    v3(0.0f, -1.0f, 0.0f),
    v3(0.0f, 0.0f, 1.0f),
    v3(0.0f, 0.0f, -1.0f)
};

static constexpr v3 CUBEMAP_UPS[6] = {
    v3(0.0f, -1.0f, 0.0f),
    v3(0.0f, -1.0f, 0.0f),
    v3(0.0f, 0.0f, 1.0f),
    v3(0.0f, 0.0f, -1.0f),
    v3(0.0f, -1.0f, 0.0f),
    v3(0.0f, -1.0f, 0.0f)
};

static constexpr Vertex AXES_VERTICES[] = {
    { .position = { 0.0f,  0.0f,  0.0f  }, .normal = { 1.0f, 0.0f, 0.0f }, .tex_coords = { 0.0f, 0.0f }},
    { .position = { 20.0f, 0.0f,  0.0f  }, .normal = { 1.0f, 0.0f, 0.0f }, .tex_coords = { 0.0f, 0.0f }},
    { .position = { 0.0f,  0.0f,  0.0f  }, .normal = { 0.0f, 1.0f, 0.0f }, .tex_coords = { 0.0f, 0.0f }},
    { .position = { 0.0f,  20.0f, 0.0f  }, .normal = { 0.0f, 1.0f, 0.0f }, .tex_coords = { 0.0f, 0.0f }},
    { .position = { 0.0f,  0.0f,  0.0f  }, .normal = { 0.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 0.0f }},
    { .position = { 0.0f,  0.0f,  20.0f }, .normal = { 0.0f, 0.0f, 1.0f }, .tex_coords = { 0.0f, 0.0f }},
};

static constexpr Vertex SCREENQUAD_VERTICES[] = {
    { .position = {-1.0f,  1.0f, 0.0f}, .normal = { 0.0f, 0.0f, 0.0f }, .tex_coords = { 0.0f, 1.0f }},
    { .position = {-1.0f, -1.0f, 0.0f}, .normal = { 0.0f, 0.0f, 0.0f }, .tex_coords = { 0.0f, 0.0f }},
    { .position = { 1.0f, -1.0f, 0.0f}, .normal = { 0.0f, 0.0f, 0.0f }, .tex_coords = { 1.0f, 0.0f }},
    { .position = {-1.0f,  1.0f, 0.0f}, .normal = { 0.0f, 0.0f, 0.0f }, .tex_coords = { 0.0f, 1.0f }},
    { .position = { 1.0f, -1.0f, 0.0f}, .normal = { 0.0f, 0.0f, 0.0f }, .tex_coords = { 1.0f, 0.0f }},
    { .position = { 1.0f,  1.0f, 0.0f}, .normal = { 0.0f, 0.0f, 0.0f }, .tex_coords = { 1.0f, 1.0f }},
};

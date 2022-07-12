// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

class drawinfo {
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
};

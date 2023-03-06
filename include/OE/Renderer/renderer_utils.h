#ifndef NRE_RENDERERUTILS_H
#define NRE_RENDERERUTILS_H

#include <OE/Renderer/DataHandler/data_handler.h>

namespace nre {

    struct render_group {

        // data for the z prepass
        nre::gpu::vertex_shader_t vs_z_prepass;
        std::size_t               z_prepass_program{0};
        bool                      isZPrePassSetup{false};

        // data for normal render
        nre::gpu::vertex_shader_t vs;
        nre::gpu::pixel_shader_t  fs;
        std::size_t               program{0};
        bool                      isSetup{false};

        // draw call data
        std::size_t camera{0};
        std::size_t material{0};
        std::size_t vgroup{0};
        std::size_t mesh{0};

        std::vector<std::size_t> lights;

        // for sorting draw calls
        bool operator<(const render_group&) const;
    };


    struct point_light_drawcall {

        point_light_drawcall() = default;
        point_light_drawcall(std::size_t, float, std::size_t);
        std::size_t id{0};
        float       z{0.0f};
        std::size_t priority{0};

        bool operator>(const point_light_drawcall&) const;
    };
}; // namespace nre
#endif

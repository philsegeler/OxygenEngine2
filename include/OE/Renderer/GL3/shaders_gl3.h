#ifndef NRE_GL3_SHADERS_H
#define NRE_GL3_SHADERS_H

#include <OE/Renderer/shaders_gpu.h>

namespace nre { namespace gl3 {
    std::string gen_vertex_shader(nre::gpu::vertex_shader_t);
    std::string gen_pixel_shader(nre::gpu::pixel_shader_t);
}; }; // namespace nre::gl3

#endif

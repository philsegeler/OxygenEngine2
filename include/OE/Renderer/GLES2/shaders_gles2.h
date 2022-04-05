#ifndef NRE_GLES2_SHADERS_H
#define NRE_GLES2_SHADERS_H

#include <OE/Renderer/shaders_gpu.h>

namespace nre { namespace gles2 {
    std::string gen_vertex_shader(nre::gpu::vertex_shader_t);
    std::string gen_pixel_shader(nre::gpu::pixel_shader_t);
}; }; // namespace nre::gles2

#endif

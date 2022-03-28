#ifndef NRE_GL3_SHADERS_H
#define NRE_GL3_SHADERS_H

#include <Renderer/NRE_GPU_Shaders.h>


std::string NRE_GenGL3VertexShader(nre::gpu::vertex_shader);
std::string NRE_GenGL3PixelShader(nre::gpu::pixel_shader);

#endif

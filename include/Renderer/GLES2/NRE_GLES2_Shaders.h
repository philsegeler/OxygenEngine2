#ifndef NRE_GLES2_SHADERS_H
#define NRE_GLES2_SHADERS_H

#include <Renderer/NRE_GPU_Shaders.h>


std::string NRE_GenGLES2VertexShader(nre::gpu::vertex_shader);
std::string NRE_GenGLES2PixelShader(nre::gpu::pixel_shader);

#endif 

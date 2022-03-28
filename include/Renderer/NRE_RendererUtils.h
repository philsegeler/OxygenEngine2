#ifndef NRE_RENDERERUTILS_H
#define NRE_RENDERERUTILS_H

#include <Renderer/DataHandler/NRE_DataHandler.h>

struct NRE_RenderGroup{
    
    // data for the z prepass
    nre::gpu::vertex_shader vs_z_prepass;
    std::size_t     z_prepass_program{0};
    bool isZPrePassSetup{false};
    
    // data for normal render
    nre::gpu::vertex_shader vs;
    nre::gpu::pixel_shader fs;    
    std::size_t     program{0};
    bool isSetup{false};
    
    // draw call data
    std::size_t     camera{0};
    std::size_t     material{0};
    std::size_t     vgroup{0};
    std::size_t     mesh{0};  
    
    std::vector<std::size_t>     lights;
    
    // for sorting draw calls
    bool operator < (const NRE_RenderGroup&) const;
};


struct NRE_PointLightDrawCall{
    
    NRE_PointLightDrawCall() = default;
    NRE_PointLightDrawCall(std::size_t, float, std::size_t);
    std::size_t id{0};
    float       z{0.0f};
    std::size_t priority{0};
    
    bool operator > (const NRE_PointLightDrawCall&) const;
};

#endif

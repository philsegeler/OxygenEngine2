#ifndef NRE_RENDERERLEGACY_H
#define NRE_RENDERERLEGACY_H

#include <OE/Renderer/api_gpu.h>
#include <OE/Renderer/renderer_utils.h>
#include <OE/dummy_classes.h>

class NRE_RendererLegacy : public oe::renderer_base_t {
public:
    NRE_RendererLegacy();
    ~NRE_RendererLegacy();

    bool init();
    bool updateSingleThread();
    bool updateData();

    bool updateMultiThread(OE_Task*, int);
    void destroy();

    // holds all rendering data from the engine
    nre::data_handler_t data_;

    // normal and point light draw calls, framebuffer, integer texture and ubo/program
    std::set<NRE_PointLightDrawCall, std::greater<NRE_PointLightDrawCall>> pt_visible_lights;
    std::map<std::size_t, NRE_DrawCallContainer>                           sce_ren_groups;

    std::size_t pt_light_ubo{0};
    std::size_t prog_light{0};

    std::size_t fbo_light{0};
    std::size_t tex_light{0};

    // bounding box and sphere data
    std::size_t prog_bbox{0};
    std::size_t vbo_bbox{0};
    std::size_t vao_bbox{0};

    std::size_t prog_sphere{0};
    std::size_t vbo_sphere{0};
    std::size_t vao_sphere{0};
    std::size_t ibo_sphere{0};

    // data for offscreen framebuffer
    std::size_t framebuffer{0};
    std::size_t colortexture{0};
    std::size_t depthrbo{0};

    // data for fullscreen quad
    std::size_t vbo_fullscreen_quad{0};
    std::size_t vao_fullscreen_quad{0};

    // gamma correction program
    std::size_t gamma_cor_prog{0};

    bool setup_bbox_prog{false};
    bool setup_sphere_prog{false};

protected:
    void initOffscreenFrameBuffer();
    void initFullscreenQuad();
    void initGammaCorrectionProg();
    void initGPUSphere();

    /// These two are in the NRE_RendererUtils.cpp
    void generateDrawCalls();
    void sortPointLights(std::size_t, std::size_t);
    /////////////////////////

    void drawRenderGroup(NRE_RenderGroup&);
    void drawRenderGroupZPrePass(NRE_RenderGroup&);
    void drawRenderGroupBoundingBox(NRE_RenderGroup&);
    void drawRenderGroupBoundingSphere(NRE_RenderGroup&);

    void setupBoundingBoxProgram();
    void setupBoundingSphereProgram();
    void initLightUBOProgramFBO();

    void updateMeshGPUData();
    void updateMaterialGPUData();
    void updateCameraGPUData();
    void updateLightGPUData();
};

#endif

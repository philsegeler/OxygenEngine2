#ifndef NRE_RENDERERLEGACY_H
#define NRE_RENDERERLEGACY_H

#include <OE/Renderer/api_gpu.h>
#include <OE/Renderer/renderer_utils.h>
#include <OE/dummy_classes.h>

class NRE_RendererLegacy : public oe::renderer_base_t {
public:
    NRE_RendererLegacy();
    ~NRE_RendererLegacy();

    bool init(oe::renderer_init_info, oe::renderer_update_info, oe::winsys_output);
    bool update_single_thread(oe::renderer_update_info, oe::winsys_output);
    // last bool is true if the renderer has been restarted. This is useful so as to fetch all the data again
    bool update_data(oe::renderer_update_info, oe::winsys_output, bool);

    void destroy();

private:
    // holds all rendering data from the engine
    nre::data_handler_t data_;

    // normal and point light draw calls, framebuffer, integer texture and ubo/program
    std::set<NRE_PointLightDrawCall, std::greater<NRE_PointLightDrawCall>> pt_visible_lights_;
    std::map<std::size_t, NRE_DrawCallContainer>                           sce_ren_groups_;

    std::size_t pt_light_ubo_{0};
    std::size_t prog_light_{0};

    std::size_t fbo_light_{0};
    std::size_t tex_light_{0};

    // bounding box and sphere data
    std::size_t prog_bbox_{0};
    std::size_t vbo_bbox_{0};
    std::size_t vao_bbox_{0};

    std::size_t prog_sphere_{0};
    std::size_t vbo_sphere_{0};
    std::size_t vao_sphere_{0};
    std::size_t ibo_sphere_{0};

    // data for offscreen framebuffer
    std::size_t framebuffer_{0};
    std::size_t colortexture_{0};
    std::size_t depthrbo_{0};

    // data for fullscreen quad
    std::size_t vbo_fullscreen_quad_{0};
    std::size_t vao_fullscreen_quad_{0};

    // gamma correction program
    std::size_t gamma_cor_prog_{0};

    bool setup_bbox_prog_{false};
    bool setup_sphere_prog_{false};

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

    oe::RENDERER_SHADING_MODE shading_mode_{oe::RENDERER_REGULAR_SHADING};
    bool                      use_wireframe_{false};
    bool                      render_bounding_boxes_{false};
    bool                      render_bounding_spheres_{false};
    bool                      use_HDR_{false};
    bool                      use_z_prepass_{false};

    int                    res_x_{0};
    int                    res_y_{0};
    oe::renderer_init_info init_info_;
};

#endif

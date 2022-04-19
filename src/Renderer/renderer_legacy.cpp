#include <OE/Renderer/renderer_legacy.h>
#include <OE/math_oe.h>
#include <iostream>

using namespace std;

NRE_RendererLegacy::NRE_RendererLegacy() {
}

NRE_RendererLegacy::~NRE_RendererLegacy() {
}

bool NRE_RendererLegacy::init(oe::renderer_init_info init_info, oe::renderer_update_info update_info,
                              oe::winsys_output winsys_info) {


    // make sure there are no stored objects
    /*cout << "NRE Cameras: " << data_.cameras_.size() << endl;
    cout << "NRE Materials: " << data_.materials_.size() << endl;
    cout << "NRE Meshes: " << data_.meshes_.size() << endl;
    cout << "NRE Vgroups: " << data_.vgroups_.size() << endl;*/

    // make sure we use the right API
    this->init_info = init_info;
    nre::gpu::init(winsys_info.backend, winsys_info.major, winsys_info.minor);

    this->initOffscreenFrameBuffer();

    this->initFullscreenQuad();

    this->initGammaCorrectionProg();

    this->initGPUSphere();
    return true;
}

void NRE_RendererLegacy::initOffscreenFrameBuffer() {
    // setup offscreen framebuffer

    this->framebuffer  = nre::gpu::new_framebuffer();
    this->colortexture = nre::gpu::new_texture();
    this->depthrbo     = nre::gpu::new_renderbuffer();

    if (this->use_HDR == false) {
        nre::gpu::set_texture_format(this->colortexture, nre::gpu::RGBA, nre::gpu::NEAREST, res_x_, res_y_, 0);
        // nre::gpu::set_texture_format(this->depthtexture, nre::gpu::DEPTHSTENCIL, nre::gpu::NEAREST,
        // res_x_, res_y_, 0);
        nre::gpu::set_renderbuffer_textype(this->depthrbo, nre::gpu::DEPTHSTENCIL, res_x_, res_y_);
    }
    else {
        nre::gpu::set_texture_format(this->colortexture, nre::gpu::RGBA16F, nre::gpu::NEAREST, res_x_, res_y_, 0);
        // nre::gpu::set_texture_format(this->depthtexture, nre::gpu::DEPTHSTENCIL, nre::gpu::NEAREST,
        // res_x_, res_y_, 0);
        nre::gpu::set_renderbuffer_textype(this->depthrbo, nre::gpu::DEPTHSTENCIL, res_x_, res_y_);
    }

    nre::gpu::set_framebuffer_texture(this->framebuffer, this->colortexture, 0);
    nre::gpu::set_framebuffer_renderbuffer(this->framebuffer, this->depthrbo, 0);
}

void NRE_RendererLegacy::initFullscreenQuad() {
    // setup fullscreen quad
    this->vbo_fullscreen_quad = nre::gpu::new_vertex_buf();
    this->vao_fullscreen_quad = nre::gpu::new_vertex_layout();

    nre::gpu::set_vertex_buf_memory_and_data(this->vbo_fullscreen_quad,
                                             {1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f},
                                             nre::gpu::STATIC);

    typedef nre::gpu::vertex_layout_input VLI; // for clarity
    std::vector<VLI>                      vao_quad_data;
    vao_quad_data.push_back(VLI(this->vbo_fullscreen_quad, 0, 2, 2));
    nre::gpu::set_vertex_layout_format(this->vao_fullscreen_quad, vao_quad_data);
}

void NRE_RendererLegacy::initGammaCorrectionProg() {
    // setup gamma correction program
    this->gamma_cor_prog = nre::gpu::new_program();

    nre::gpu::vertex_shader_t vs_gamma;
    nre::gpu::pixel_shader_t  fs_gamma;

    vs_gamma.type           = nre::gpu::VS_UNDEFINED;
    vs_gamma.fullscreenQuad = true;
    fs_gamma.type           = nre::gpu::FS_GAMMA;

    nre::gpu::set_program_vertex_shader(this->gamma_cor_prog, vs_gamma);
    nre::gpu::set_program_pixel_shader(this->gamma_cor_prog, fs_gamma);

    nre::gpu::setup_program(this->gamma_cor_prog);
    // nre::gpu::setProgramTextureSlot(this->gamma_cor_prog, "tex_output", 0);
}

void NRE_RendererLegacy::initGPUSphere() {
    // the same VBO, VAO and IBO are used for every object
    // it holds a sphere with radius one

    this->vbo_sphere = nre::gpu::new_vertex_buf();
    this->ibo_sphere = nre::gpu::new_index_buf();

    auto sphere_vbo_data = OE_GetBoundingSphereVertexBuffer(1.0f, 1.0f, 16);
    auto sphere_ibo_data = OE_GetBoundingSphereIndexBuffer(1.0f, 1.0f, 16);

    nre::gpu::set_vertex_buf_memory_and_data(this->vbo_sphere, sphere_vbo_data, nre::gpu::STATIC);
    nre::gpu::set_index_buf_memory_and_data(this->ibo_sphere, sphere_ibo_data, nre::gpu::STATIC);

    typedef nre::gpu::vertex_layout_input VLI; // for clarity
    this->vao_sphere = nre::gpu::new_vertex_layout();

    std::vector<VLI> vao_sphere_data;
    vao_sphere_data.push_back(VLI(this->vbo_sphere, 0, 3, 6));
    vao_sphere_data.push_back(VLI(this->vbo_sphere, 3, 3, 6));

    nre::gpu::set_vertex_layout_format(this->vao_sphere, vao_sphere_data);
}

//------------------------updateData---------------------------//

bool NRE_RendererLegacy::update_data(oe::renderer_update_info update_info, oe::winsys_output winsys_info,
                                     bool has_renderer_restarted) {
    assert(this->world != nullptr);
    res_x_ = winsys_info.res_x;
    res_y_ = winsys_info.res_y;

    this->use_wireframe           = update_info.use_wireframe;
    this->render_bounding_boxes   = update_info.render_bounding_boxes;
    this->render_bounding_spheres = update_info.render_bounding_spheres;
    this->use_HDR                 = update_info.use_hdr;
    this->use_z_prepass           = update_info.use_z_prepass;
    bool temp_restart_renderer    = (this->shading_mode != update_info.shading_mode);

    if (temp_restart_renderer) {
        this->destroy();
        this->init(this->init_info, update_info, winsys_info);
    }
    else if (has_renderer_restarted) {
        this->init(this->init_info, update_info, winsys_info);
    }
    this->shading_mode = update_info.shading_mode;
    data_.update(temp_restart_renderer or has_renderer_restarted, this->render_bounding_boxes or this->render_bounding_spheres);

    return true;
}

//------------------------updateSIngleThread-------------------//

bool NRE_RendererLegacy::update_single_thread(oe::renderer_update_info update_info, oe::winsys_output winsys_info) {

    res_x_ = winsys_info.res_x;
    res_y_ = winsys_info.res_y;

    this->use_wireframe           = update_info.use_wireframe;
    this->render_bounding_boxes   = update_info.render_bounding_boxes;
    this->render_bounding_spheres = update_info.render_bounding_spheres;
    this->use_HDR                 = update_info.use_hdr;
    this->use_z_prepass           = update_info.use_z_prepass;

    nre::gpu::update(res_x_, res_y_, update_info.sanity_checks);

    if (not this->use_HDR) {
        nre::gpu::set_texture_format(this->colortexture, nre::gpu::RGBA, nre::gpu::NEAREST, res_x_, res_y_, 0);
        // nre::gpu::set_texture_format(this->depthtexture, nre::gpu::DEPTHSTENCIL, nre::gpu::NEAREST,
        // res_x_, res_y_, 0);
        nre::gpu::set_renderbuffer_textype(this->depthrbo, nre::gpu::DEPTHSTENCIL, res_x_, res_y_);
    }
    else {
        nre::gpu::set_texture_format(this->colortexture, nre::gpu::RGBA16F, nre::gpu::NEAREST, res_x_, res_y_, 0);
        // nre::gpu::set_texture_format(this->depthtexture, nre::gpu::DEPTHSTENCIL, nre::gpu::NEAREST,
        // res_x_, res_y_, 0);
        nre::gpu::set_renderbuffer_textype(this->depthrbo, nre::gpu::DEPTHSTENCIL, res_x_, res_y_);
    }
    // update light texture
    // nre::gpu::set_texture_format(this->tex_light, nre::gpu::RGBA, nre::gpu::NEAREST, res_x_,
    // res_y_, 0);

    // generate draw calls
    this->generateDrawCalls();

    // upload all remaining data to the GPU
    // and clean up any unused data
    this->updateMeshGPUData();
    this->updateMaterialGPUData();
    this->updateCameraGPUData();


    // render viewport
    nre::gpu::use_wireframe(this->use_wireframe);

    /*cout << "NRE Cameras: " << data_.cameras_.size() << endl;
    cout << "NRE Materials: " << data_.materials_.size() << endl;
    cout << "NRE Meshes: " << data_.meshes_.size() << endl;
    cout << "NRE Vgroups: " << data_.vgroups_.size() << endl;*/

    nre::gpu::use_framebuffer(this->framebuffer);
    nre::gpu::clear_framebuffer(this->framebuffer, nre::gpu::FBO_ALL, 1.0f);

    if (data_.loaded_viewport_ != 0) {

        auto lv = data_.loaded_viewport_;

        if (data_.viewports_[lv].cameras.size() == 0) {
            throw nre::incomplete_viewport(lv);
        }
        // TEMPORARY until multiple layers and cameras support is implemented
        else if (data_.viewports_[lv].cameras.size() > 1) {
            throw nre::unsupported_viewport(lv, "Too many (" + to_string(data_.viewports_[lv].cameras.size()) + ") cameras");
        }
        else {
        }

        // only one fullscreen camera is supported for now
        auto camera_id = data_.viewports_[lv].cameras[0];
        auto scene_id  = data_.cameras_[camera_id].scene_id;

        // draw everything required for the z prepass, which also populates the depth buffer
        if (update_info.use_z_prepass) {
            nre::gpu::set_render_mode(nre::gpu::Z_PREPASS_BACKFACE);
            for (auto x : this->sce_ren_groups[scene_id]) {
                if (x.camera == camera_id) {
                    this->drawRenderGroupZPrePass(x);
                    this->sce_ren_groups[scene_id].replace(x);
                }
            }
            this->sce_ren_groups[scene_id].update();
        }

        // sort and draw lights
        this->sortPointLights(scene_id, camera_id);

        // draw everything normally
        nre::gpu::use_framebuffer(this->framebuffer);
        if (update_info.use_z_prepass) {
            nre::gpu::set_render_mode(nre::gpu::AFTERPREPASS_BACKFACE);
        }
        else {
            nre::gpu::set_render_mode(nre::gpu::REGULAR_BACKFACE);
        }

        for (auto x : this->sce_ren_groups[scene_id]) {
            if (x.camera == camera_id) {
                this->drawRenderGroup(x);
                this->sce_ren_groups[scene_id].replace(x);
            }
        }
        this->sce_ren_groups[scene_id].update();

        // optionally draw a bounding box/sphere for each object (in wireframe mode)
        bool render_bboxes  = this->render_bounding_boxes;
        bool render_spheres = this->render_bounding_spheres;
        nre::gpu::use_wireframe(true);

        if (render_bboxes) {
            nre::gpu::set_render_mode(nre::gpu::REGULAR_BOTH);
            if (!this->setup_bbox_prog) {

                this->setupBoundingBoxProgram();
                this->setup_bbox_prog = true;
            }
            for (auto x : this->sce_ren_groups[scene_id]) {
                if (x.camera == camera_id) {
                    this->drawRenderGroupBoundingBox(x);
                    this->sce_ren_groups[scene_id].replace(x);
                }
            }
            this->sce_ren_groups[scene_id].update();
        }
        if (render_spheres) {
            nre::gpu::set_render_mode(nre::gpu::REGULAR_BOTH);
            if (!this->setup_sphere_prog) {

                this->setupBoundingSphereProgram();
                this->setup_sphere_prog = true;
            }
            for (auto x : this->sce_ren_groups[scene_id]) {
                if (x.camera == camera_id) {
                    this->drawRenderGroupBoundingSphere(x);
                    this->sce_ren_groups[scene_id].replace(x);
                }
            }
            this->sce_ren_groups[scene_id].update();
        }
    }

    // nre::gpu::copyFrameBuffer(this->framebuffer, 0, nre::gpu::FBO_DEPTHSTENCIL);

    nre::gpu::use_framebuffer(0);
    nre::gpu::use_wireframe(false);
    nre::gpu::set_render_mode(nre::gpu::FULLSCREEN_QUAD);

    nre::gpu::set_texture_slot(this->colortexture, 0);

    nre::gpu::draw(this->gamma_cor_prog, this->vao_fullscreen_quad);
    return true;
}

void NRE_RendererLegacy::drawRenderGroup(NRE_RenderGroup& ren_group) {

    if (!ren_group.isSetup) {
        // cout << "Setting up Render group" << ren_group.camera << " " << ren_group.vgroup << " " << ren_group.mesh << endl;

        ren_group.isSetup = true;

        ren_group.vs            = nre::gpu::vertex_shader_t();
        ren_group.vs.type       = nre::gpu::VS_REGULAR;
        ren_group.vs.num_of_uvs = data_.meshes_[ren_group.mesh].uvmaps;


        // choose shading mode
        ren_group.fs = nre::gpu::pixel_shader_t();
        lockMutex();
        switch (this->shading_mode) {
        case oe::RENDERER_NORMALS_SHADING:
            ren_group.fs.type = nre::gpu::FS_NORMALS;
            break;
        case oe::RENDERER_NO_LIGHTS_SHADING:
            ren_group.fs.type = nre::gpu::FS_MATERIAL;
            break;
        case oe::RENDERER_DIR_LIGHTS_SHADING:

            break;
        case oe::RENDERER_INDEXED_LIGHTS_SHADING:

            break;
        case oe::RENDERER_REGULAR_SHADING:
            ren_group.fs.type = nre::gpu::FS_MATERIAL;
            break;
        }
        unlockMutex();
        ren_group.fs.num_of_uvs = data_.meshes_[ren_group.mesh].uvmaps;

        ren_group.program = nre::gpu::new_program();
        nre::gpu::set_program_vertex_shader(ren_group.program, ren_group.vs);
        nre::gpu::set_program_pixel_shader(ren_group.program, ren_group.fs);

        nre::gpu::setup_program(ren_group.program);
    }

    nre::gpu::set_program_uniform_data(ren_group.program, "Model_Matrix", data_.meshes_[ren_group.mesh].data);
    nre::gpu::set_program_uniform_data(
        ren_group.program, "MVP_Matrix",
        OE_Mat4x4ToSTDVector(data_.cameras_[ren_group.camera].perspective_view_mat * data_.meshes_[ren_group.mesh].model_mat));
    if (nre::gpu::get_program_uniform_slot(ren_group.program, "mat_diffuse") != -2) {
        nre::gpu::set_program_uniform_data(ren_group.program, "mat_diffuse",
                                           OE_Vec4ToSTDVector(data_.materials_[ren_group.material].get_mat_diffuse()));
    }
    if (nre::gpu::get_program_uniform_slot(ren_group.program, "camera_pos") != -2) {
        nre::gpu::set_program_uniform_data(ren_group.program, "camera_pos",
                                           OE_Vec4ToSTDVector(data_.cameras_[ren_group.camera].get_position()));
    }
    if (nre::gpu::get_program_uniform_slot(ren_group.program, "mat_specular_hardness") != -2) {
        nre::gpu::set_program_uniform_data(ren_group.program, "mat_specular_hardness",
                                           data_.materials_[ren_group.material].get_mat_specular_hardness());
    }
    nre::gpu::set_program_uniform_data(ren_group.program, "Model_Matrix", data_.meshes_[ren_group.mesh].data);
    nre::gpu::draw(ren_group.program, data_.meshes_[ren_group.mesh].vao, data_.vgroups_[ren_group.vgroup].ibo);
}

void NRE_RendererLegacy::drawRenderGroupZPrePass(NRE_RenderGroup& ren_group) {

    if (!ren_group.isZPrePassSetup) {

        ren_group.isZPrePassSetup = true;

        ren_group.vs_z_prepass            = nre::gpu::vertex_shader_t();
        ren_group.vs_z_prepass.type       = nre::gpu::VS_Z_PREPASS;
        ren_group.vs_z_prepass.num_of_uvs = data_.meshes_[ren_group.mesh].uvmaps;

        ren_group.z_prepass_program = nre::gpu::new_program();

        nre::gpu::set_program_vertex_shader(ren_group.z_prepass_program, ren_group.vs_z_prepass);

        nre::gpu::setup_program(ren_group.z_prepass_program);
    }
    nre::gpu::set_program_uniform_data(
        ren_group.z_prepass_program, "MVP_Matrix",
        OE_Mat4x4ToSTDVector(data_.cameras_[ren_group.camera].perspective_view_mat * data_.meshes_[ren_group.mesh].model_mat));
    nre::gpu::draw(ren_group.z_prepass_program, data_.meshes_[ren_group.mesh].vao, data_.vgroups_[ren_group.vgroup].ibo);
}

void NRE_RendererLegacy::drawRenderGroupBoundingBox(NRE_RenderGroup& ren_group) {

    nre::gpu::set_program_uniform_data(this->prog_bbox, "Model_Matrix", data_.meshes_[ren_group.mesh].data);
    nre::gpu::set_program_uniform_data(this->prog_bbox, "PV_Matrix",
                                       OE_Mat4x4ToSTDVector(data_.cameras_[ren_group.camera].perspective_view_mat));
    nre::gpu::set_program_uniform_data(this->prog_bbox, "scaling_min_data",
                                       data_.meshes_[ren_group.mesh].get_scaling_min_data());
    nre::gpu::set_program_uniform_data(this->prog_bbox, "scaling_max_data",
                                       data_.meshes_[ren_group.mesh].get_scaling_max_data());
    nre::gpu::draw(this->prog_bbox, this->vao_bbox);
}

void NRE_RendererLegacy::drawRenderGroupBoundingSphere(NRE_RenderGroup& ren_group) {

    nre::gpu::set_program_uniform_data(this->prog_sphere, "Model_Matrix", data_.meshes_[ren_group.mesh].data);
    nre::gpu::set_program_uniform_data(this->prog_sphere, "PV_Matrix",
                                       OE_Mat4x4ToSTDVector(data_.cameras_[ren_group.camera].perspective_view_mat));
    nre::gpu::set_program_uniform_data(this->prog_sphere, "scaling_min_data",
                                       data_.meshes_[ren_group.mesh].get_scaling_min_data());
    nre::gpu::set_program_uniform_data(this->prog_sphere, "scaling_max_data",
                                       data_.meshes_[ren_group.mesh].get_scaling_max_data());
    nre::gpu::draw(this->prog_sphere, this->vao_sphere, this->ibo_sphere);
}


void NRE_RendererLegacy::setupBoundingBoxProgram() {

    // the same vbo and vao are used for every object
    // The vbo holds a cube with side 1 that is scaled appropriately in the shader
    // through the uniform block variable 'scaling_data'

    this->vbo_bbox = nre::gpu::new_vertex_buf();
    auto bbox_data = OE_GetBoundingBoxVertexBuffer(1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f);
    nre::gpu::set_vertex_buf_memory_and_data(this->vbo_bbox, bbox_data, nre::gpu::STATIC);

    typedef nre::gpu::vertex_layout_input VLI; // for clarity
    this->vao_bbox = nre::gpu::new_vertex_layout();

    std::vector<VLI> vao_bbox_data;
    vao_bbox_data.push_back(VLI(this->vbo_bbox, 0, 3, 6));
    vao_bbox_data.push_back(VLI(this->vbo_bbox, 3, 3, 6));

    nre::gpu::set_vertex_layout_format(this->vao_bbox, vao_bbox_data);

    this->prog_bbox = nre::gpu::new_program();

    nre::gpu::vertex_shader_t vs_bbox;
    nre::gpu::pixel_shader_t  fs_bbox;

    vs_bbox.type       = nre::gpu::VS_BOUNDING_BOX;
    vs_bbox.num_of_uvs = 0;

    fs_bbox.type       = nre::gpu::FS_NORMALS;
    fs_bbox.num_of_uvs = 0;

    nre::gpu::set_program_vertex_shader(this->prog_bbox, vs_bbox);
    nre::gpu::set_program_pixel_shader(this->prog_bbox, fs_bbox);

    nre::gpu::setup_program(this->prog_bbox);
}

void NRE_RendererLegacy::setupBoundingSphereProgram() {

    this->prog_sphere = nre::gpu::new_program();

    nre::gpu::vertex_shader_t vs_sphere;
    nre::gpu::pixel_shader_t  fs_sphere;

    vs_sphere.type       = nre::gpu::VS_BOUNDING_SPHERE;
    vs_sphere.num_of_uvs = 0;

    fs_sphere.type       = nre::gpu::FS_NORMALS;
    fs_sphere.num_of_uvs = 0;

    nre::gpu::set_program_vertex_shader(this->prog_sphere, vs_sphere);
    nre::gpu::set_program_pixel_shader(this->prog_sphere, fs_sphere);

    nre::gpu::setup_program(this->prog_sphere);
}

//---------------------------------Update actual GPU data------------------------------//

void NRE_RendererLegacy::updateMeshGPUData() {
    for (auto mesh : data_.meshes_) {

        // first time buffer initialization
        if (!data_.meshes_[mesh.first].has_init) {
            data_.meshes_[mesh.first].vbo      = nre::gpu::new_vertex_buf();
            data_.meshes_[mesh.first].vao      = nre::gpu::new_vertex_layout();
            data_.meshes_[mesh.first].has_init = true;
        }


        // initialize data itself
        if (!data_.meshes_[mesh.first].vao_initialized) {

            /// vertex buffer
            data_.meshes_[mesh.first].mesh->data->vbo_mutex.lockMutex();
            nre::gpu::set_vertex_buf_memory_and_data(data_.meshes_[mesh.first].vbo, data_.meshes_[mesh.first].mesh->data->vbo,
                                                     nre::gpu::STATIC);
            data_.meshes_[mesh.first].mesh->data->vbo.clear();
            data_.meshes_[mesh.first].mesh->data->vbo_mutex.unlockMutex();


            /// index buffers
            data_.meshes_[mesh.first].mesh->data->ibos_mutex.lockMutex();
            for (auto vg : mesh.second.vgroups) {
                data_.vgroups_[vg].ibo = nre::gpu::new_index_buf();
                nre::gpu::set_index_buf_memory_and_data(data_.vgroups_[vg].ibo,
                                                        data_.meshes_[mesh.first].mesh->data->ibos[vg].data, nre::gpu::STATIC);
                data_.meshes_[mesh.first].mesh->data->ibos[vg].data.clear();
            }

            data_.meshes_[mesh.first].mesh->data->ibos_mutex.unlockMutex();

            /// vertex layout
            typedef nre::gpu::vertex_layout_input VLI; // for clarity

            data_.meshes_[mesh.first].mesh->data->ibos.clear();
            // delete data_.meshes_[mesh.first].mesh->data->index_buffer;
            // data_.meshes_[mesh.first].mesh->data->index_buffer = nullptr;

            data_.meshes_[mesh.first].vao_input.clear();

            data_.meshes_[mesh.first].vao_input.push_back(
                VLI(data_.meshes_[mesh.first].vbo, 0, 3, 6 + data_.meshes_[mesh.first].uvmaps * 2));
            data_.meshes_[mesh.first].vao_input.push_back(
                VLI(data_.meshes_[mesh.first].vbo, 3, 3, 6 + data_.meshes_[mesh.first].uvmaps * 2));

            for (size_t i = 0; i < data_.meshes_[mesh.first].uvmaps; i++) {
                data_.meshes_[mesh.first].vao_input.push_back(
                    VLI(data_.meshes_[mesh.first].vbo, 6 + 2 * i, 2, 6 + data_.meshes_[mesh.first].uvmaps * 2));
            }

            nre::gpu::set_vertex_layout_format(data_.meshes_[mesh.first].vao, data_.meshes_[mesh.first].vao_input);
            data_.meshes_[mesh.first].vao_initialized = true;
        }

        // update per frame data
        if (data_.meshes_[mesh.first].changed) {


            data_.meshes_[mesh.first].data = OE_Mat4x4ToSTDVector(data_.meshes_[mesh.first].model_mat);

            // populate scaling_max_data
            data_.meshes_[mesh.first].data.push_back(data_.meshes_[mesh.first].max_x);
            data_.meshes_[mesh.first].data.push_back(data_.meshes_[mesh.first].max_y);
            data_.meshes_[mesh.first].data.push_back(data_.meshes_[mesh.first].max_z);

            // final element is sphere radius candidate
            data_.meshes_[mesh.first].data.push_back(data_.meshes_[mesh.first].max_radius);

            // populate scaling_min_data
            data_.meshes_[mesh.first].data.push_back(data_.meshes_[mesh.first].min_x);
            data_.meshes_[mesh.first].data.push_back(data_.meshes_[mesh.first].min_y);
            data_.meshes_[mesh.first].data.push_back(data_.meshes_[mesh.first].min_z);

            // final element is sphere radius candidate
            data_.meshes_[mesh.first].data.push_back(data_.meshes_[mesh.first].min_radius);

            if (data_.meshes_[mesh.first].size != data_.meshes_[mesh.first].data.size()) {
                data_.meshes_[mesh.first].size = data_.meshes_[mesh.first].data.size();
            }

            data_.meshes_[mesh.first].changed = false;
        }
    }

    for (auto cam : data_.deleted_meshes_) {
        this->sce_ren_groups[data_.meshes_[cam].scene_id].removeMesh(cam);
        data_.delete_mesh(cam);
    }
    data_.deleted_meshes_.clear();
}

void NRE_RendererLegacy::updateMaterialGPUData() {
    for (auto mat : data_.materials_) {

        if (!data_.materials_[mat.first].has_init) {
            data_.materials_[mat.first].has_init = true;
        }

        if (data_.materials_[mat.first].changed) {

            if (data_.materials_[mat.first].size != data_.materials_[mat.first].data.size()) {
                data_.materials_[mat.first].size = data_.materials_[mat.first].data.size();
            }
            data_.materials_[mat.first].changed = false;
        }
    }

    for (auto cam : data_.deleted_materials_) {
        this->sce_ren_groups[data_.materials_[cam].scene_id].removeMaterial(cam);
        data_.delete_material(cam);
    }
    data_.deleted_materials_.clear();
}

void NRE_RendererLegacy::updateCameraGPUData() {
    for (auto cam : data_.cameras_) {

        if (!data_.cameras_[cam.first].has_init) {
            data_.cameras_[cam.first].has_init = true;
        }
        if (data_.cameras_[cam.first].changed) {

            if (data_.cameras_[cam.first].size != data_.cameras_[cam.first].data.size()) {
                data_.cameras_[cam.first].size = data_.cameras_[cam.first].data.size();
            }
            data_.cameras_[cam.first].changed = false;
        }
    }

    for (auto cam : data_.deleted_cameras_) {
        this->sce_ren_groups[data_.cameras_[cam].scene_id].removeCamera(cam);
        data_.delete_camera(cam);
    }
    data_.deleted_cameras_.clear();
}
void NRE_RendererLegacy::updateLightGPUData() {
    /*for (auto l: data_.pt_lights){

        if (!data_.pt_lights[l.first].has_init){
            data_.pt_lights[l.first].ubo = nre::gpu::new_uniform_buf();
            data_.pt_lights[l.first].has_init = true;
        }
        if (data_.pt_lights[l.first].changed){

            if (data_.pt_lights[l.first].size != data_.pt_lights[l.first].data.size()){
                data_.pt_lights[l.first].size = data_.pt_lights[l.first].data.size();
                nre::gpu::set_uniform_buf_memory(data_.pt_lights[l.first].ubo, data_.pt_lights[l.first].size,
    nre::gpu::DYNAMIC);
            }

            nre::gpu::set_uniform_buf_data(data_.pt_lights[l.first].ubo, data_.pt_lights[l.first].data, 0);
            data_.pt_lights[l.first].changed = false;
        }
    }*/
    std::vector<float> pt_light_data;
    pt_light_data.reserve(16 * 255);
    int count = 0;
    for (auto l : this->pt_visible_lights) {
        pt_light_data.insert(pt_light_data.end(), data_.pt_lights_[l.id].data.begin(), data_.pt_lights_[l.id].data.end());

        count++;
        if (count > 255) break;
    }

    // fill the rest with zeros since webgl complains otherwise
    for (int i = 0; i < 16 * (255 - count); i++) {
        pt_light_data.push_back(0.0f);
    }
}

bool NRE_RendererLegacy::update_multi_thread(OE_Task*, int) {
    return false;
}

void NRE_RendererLegacy::destroy() {

    this->setup_bbox_prog   = false;
    this->setup_sphere_prog = false;

    data_.clear();
    this->sce_ren_groups.clear();

    nre::gpu::destroy();
}
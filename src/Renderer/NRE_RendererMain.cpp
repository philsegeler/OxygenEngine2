#include <OE_Math.h>
#include <Renderer/NRE_RendererMain.h>
#include <iostream>

using namespace std;

NRE_Renderer::NRE_Renderer(){
    
}

NRE_Renderer::~NRE_Renderer(){
   
}
    
bool NRE_Renderer::init(){
    
    
    // make sure there are no stored objects
    /*cout << "NRE Cameras: " << data_.cameras.size() << endl;
    cout << "NRE Materials: " << data_.materials.size() << endl;
    cout << "NRE Meshes: " << data_.meshes.size() << endl;
    cout << "NRE Vgroups: " << data_.vgroups.size() << endl;*/
    
    // make sure we use the right API
    // it should have been initialized from the window manager
    nre::gpu::init(nre::gpu::get_api(), nre::gpu::backend_info.major, nre::gpu::backend_info.minor);
    
    this->initOffscreenFrameBuffer();
    this->initFullscreenQuad();
    this->initGammaCorrectionProg();
    this->initGPUSphere();
    this->initLightUBOProgramFBO();    
    
    return true;
}

void NRE_Renderer::initOffscreenFrameBuffer(){
    // setup offscreen framebuffer
    this->framebuffer = nre::gpu::new_framebuffer();
    this->colortexture = nre::gpu::new_texture();
    this->depthrbo = nre::gpu::new_renderbuffer();
    
    if (this->use_HDR.load(std::memory_order_relaxed) == false){
        nre::gpu::set_texture_format(this->colortexture, nre::gpu::RGB10_A2, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
        //nre::gpu::set_texture_format(this->depthtexture, nre::gpu::DEPTHSTENCIL, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
        nre::gpu::set_renderbuffer_mode(this->depthrbo, nre::gpu::DEPTHSTENCIL, this->screen->resolution_x, this->screen->resolution_y);
    } else {
        nre::gpu::set_texture_format(this->colortexture, nre::gpu::RGBA16F, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
        //nre::gpu::set_texture_format(this->depthtexture, nre::gpu::DEPTHSTENCIL, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
        nre::gpu::set_renderbuffer_mode(this->depthrbo, nre::gpu::DEPTHSTENCIL, this->screen->resolution_x, this->screen->resolution_y);
    }
    
    nre::gpu::set_framebuffer_texture(this->framebuffer, this->colortexture, 0);
    //nre::gpu::set_framebuffer_texture(this->framebuffer, this->depthtexture, 0);
    nre::gpu::set_framebuffer_renderbuffer(this->framebuffer, this->depthrbo, 0);
}

void NRE_Renderer::initFullscreenQuad(){
    // setup fullscreen quad
    this->vbo_fullscreen_quad = nre::gpu::new_vertex_buf();
    this->vao_fullscreen_quad = nre::gpu::new_vertex_layout();
    
    nre::gpu::set_vertex_buf_memory_and_data(this->vbo_fullscreen_quad, {1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,-1.0f, 1.0f}, nre::gpu::STATIC);
    
    typedef nre::gpu::vertex_layout_input VLI; // for clarity
    std::vector<VLI> vao_quad_data;
    vao_quad_data.push_back(VLI(this->vbo_fullscreen_quad, 0, 2, 2));
    nre::gpu::set_vertex_layout_format(this->vao_fullscreen_quad, vao_quad_data);
}

void NRE_Renderer::initGammaCorrectionProg(){
    // setup gamma correction program
    this->gamma_cor_prog = nre::gpu::new_program();
        
    nre::gpu::vertex_shader vs_gamma;
    nre::gpu::pixel_shader fs_gamma;
        
    vs_gamma.type = nre::gpu::VS_UNDEFINED;
    vs_gamma.fullscreenQuad = true;
    fs_gamma.type = nre::gpu::FS_GAMMA;
        
    nre::gpu::set_program_vertex_shader(this->gamma_cor_prog, vs_gamma);
    nre::gpu::set_program_pixel_shader(this->gamma_cor_prog, fs_gamma);
        
    nre::gpu::setup_program(this->gamma_cor_prog);
    //nre::gpu::setProgramTextureSlot(this->gamma_cor_prog, "tex_output", 0);
}

void NRE_Renderer::initGPUSphere(){
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

void NRE_Renderer::initLightUBOProgramFBO(){
    
    // setup point light ubo
    this->pt_light_ubo = nre::gpu::new_uniform_buf();
    
    // setup light program
    this->prog_light = nre::gpu::new_program();
        
    nre::gpu::vertex_shader vs_light;
    nre::gpu::pixel_shader fs_light;
        
    vs_light.type = nre::gpu::VS_LIGHT;
    vs_light.num_of_uvs = 0;
        
    fs_light.type = nre::gpu::FS_LIGHT_INDEX;
    fs_light.num_of_uvs = 0;
        
    nre::gpu::set_program_vertex_shader(this->prog_light, vs_light);
    nre::gpu::set_program_pixel_shader(this->prog_light, fs_light);
        
    nre::gpu::setup_program(this->prog_light);
    
    nre::gpu::set_program_uniform_buf_slot(this->prog_light, "OE_Camera", 0);
    nre::gpu::set_program_uniform_buf_slot(this->prog_light, "OE_Lights", 1);
    
    // setup light own texture and framebuffer to be used for storing the indices
    this->tex_light = nre::gpu::new_texture();
    
    nre::gpu::set_texture_format(this->tex_light, nre::gpu::RGBA, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
    
    this->fbo_light = nre::gpu::new_framebuffer();
    nre::gpu::set_framebuffer_texture(this->fbo_light, this->tex_light, 0);
    nre::gpu::set_framebuffer_renderbuffer(this->fbo_light, this->depthrbo, 0);
}

//------------------------updateData---------------------------//

bool NRE_Renderer::updateData(){
    assert (this->world != nullptr);
    bool temp_restart_renderer = this->screen->restart_renderer;
    this->screen->restart_renderer = false;
    
    if (temp_restart_renderer){
        this->destroy();
        this->init();
    }
    
    data_.update(temp_restart_renderer, this->render_bounding_boxes.load(std::memory_order_relaxed) or this->render_bounding_spheres.load(std::memory_order_relaxed));
    
    return true;
}

//------------------------updateSIngleThread-------------------//

bool NRE_Renderer::updateSingleThread(){
    
    nre::gpu::update(this->screen->resolution_x, this->screen->resolution_y);
    
    if (this->use_HDR.load(std::memory_order_relaxed) == false){
        nre::gpu::set_texture_format(this->colortexture, nre::gpu::RGB10_A2, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
        //nre::gpu::set_texture_format(this->depthtexture, nre::gpu::DEPTHSTENCIL, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
        nre::gpu::set_renderbuffer_mode(this->depthrbo, nre::gpu::DEPTHSTENCIL, this->screen->resolution_x, this->screen->resolution_y);
    } else {
        nre::gpu::set_texture_format(this->colortexture, nre::gpu::RGBA16F, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
        //nre::gpu::set_texture_format(this->depthtexture, nre::gpu::DEPTHSTENCIL, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
        nre::gpu::set_renderbuffer_mode(this->depthrbo, nre::gpu::DEPTHSTENCIL, this->screen->resolution_x, this->screen->resolution_y);
    }
    // update light texture
    nre::gpu::set_texture_format(this->tex_light, nre::gpu::RGBA, nre::gpu::NEAREST, this->screen->resolution_x, this->screen->resolution_y, 0);
    
    // generate draw calls
    this->generateDrawCalls();
    
    // upload all remaining data to the GPU 
    // and clean up any unused data
    this->updateMeshGPUData();
    this->updateMaterialGPUData();
    this->updateCameraGPUData();
    
    
    // render viewport
    nre::gpu::use_wireframe = this->use_wireframe.load(std::memory_order_relaxed);
    
    /*cout << "NRE Cameras: " << data_.cameras.size() << endl;
    cout << "NRE Materials: " << data_.materials.size() << endl;
    cout << "NRE Meshes: " << data_.meshes.size() << endl;
    cout << "NRE Vgroups: " << data_.vgroups.size() << endl;*/
    
    nre::gpu::use_framebuffer(this->framebuffer);
    nre::gpu::clear_framebuffer(this->framebuffer, nre::gpu::FBO_ALL, 1.0f);

    if (data_.loaded_viewport != 0){

        auto lv = data_.loaded_viewport;
        
        if (data_.viewports[lv].cameras.size() == 0){
            throw nre::incomplete_viewport(lv);
        }
        // TEMPORARY until multiple layers and cameras support is implemented
        else if (data_.viewports[lv].cameras.size() > 1){
            throw nre::unsupported_viewport(lv, "Too many (" + to_string(data_.viewports[lv].cameras.size()) + ") cameras");
        }
        else{
            
        }
        
        // only one fullscreen camera is supported for now
        auto camera_id = data_.viewports[lv].cameras[0];
        auto scene_id = data_.cameras[camera_id].scene_id;
        
        // draw everything required for the z prepass, which also populates the depth buffer
        nre::gpu::set_render_mode(nre::gpu::Z_PREPASS_BACKFACE);
        for (auto x: this->sce_ren_groups[scene_id]){
            if (x.camera == camera_id){
                this->drawRenderGroupZPrePass(x);
                this->sce_ren_groups[scene_id].replace(x);
            }
        }
        this->sce_ren_groups[scene_id].update();
        
        //sort point lights
        this->sortPointLights(scene_id, camera_id);
        this->updateLightGPUData();
        
        nre::gpu::use_framebuffer(this->fbo_light);
        nre::gpu::clear_framebuffer(this->fbo_light, nre::gpu::FBO_COLORSTENCIL, 0.0f);
        
        nre::gpu::set_render_mode(nre::gpu::LIGHT_PREPASS_2);
        
        nre::gpu::set_uniform_buf_state(this->pt_light_ubo, this->prog_light, 1, 0, 0);
        nre::gpu::set_uniform_buf_state(data_.cameras[camera_id].ubo, this->prog_light, 0, 0, 0);    
        
        int lights_count = this->pt_visible_lights.size();
        
        nre::gpu::draw_instanced(this->prog_light, this->vao_sphere, this->ibo_sphere, std::min<int>(lights_count, 255));
        
        nre::gpu::set_render_mode(nre::gpu::LIGHT_AFTERPASS_RG);
        nre::gpu::draw_instanced(this->prog_light, this->vao_sphere, this->ibo_sphere, std::min<int>(lights_count, 255));
        
        nre::gpu::set_render_mode(nre::gpu::LIGHT_AFTERPASS_BA);
        nre::gpu::draw_instanced(this->prog_light, this->vao_sphere, this->ibo_sphere, std::min<int>(lights_count, 255));
        
        // draw everything normally
        nre::gpu::use_framebuffer(this->framebuffer);
        nre::gpu::set_render_mode(nre::gpu::AFTERPREPASS_BACKFACE);
        for (auto x: this->sce_ren_groups[scene_id]){
            if (x.camera == camera_id){
                this->drawRenderGroup(x);
                this->sce_ren_groups[scene_id].replace(x);
            }
        }
        this->sce_ren_groups[scene_id].update();
    
        // optionally draw a bounding box/sphere for each object (in wireframe mode)
        bool temp = nre::gpu::use_wireframe;
        bool render_bboxes = this->render_bounding_boxes.load(std::memory_order_relaxed);
        bool render_spheres = this->render_bounding_spheres.load(std::memory_order_relaxed);
        nre::gpu::use_wireframe = true;
    
        if (render_bboxes){
            nre::gpu::set_render_mode(nre::gpu::REGULAR_BOTH);
            if (!this->setup_bbox_prog){
        
                this->setupBoundingBoxProgram();
                this->setup_bbox_prog = true;
            }
            for (auto x: this->sce_ren_groups[scene_id]){
                if (x.camera == camera_id){
                    this->drawRenderGroupBoundingBox(x);
                    this->sce_ren_groups[scene_id].replace(x);
                }
            }
            this->sce_ren_groups[scene_id].update();
        }
        if (render_spheres){
            nre::gpu::set_render_mode(nre::gpu::REGULAR_BOTH);
            if (!this->setup_sphere_prog){
        
                this->setupBoundingSphereProgram();
                this->setup_sphere_prog = true;
            }
            for (auto x: this->sce_ren_groups[scene_id]){
                if (x.camera == camera_id){
                    this->drawRenderGroupBoundingSphere(x);
                    this->sce_ren_groups[scene_id].replace(x);
                }
            }
            this->sce_ren_groups[scene_id].update();
        }
        
        nre::gpu::use_wireframe = temp;
    }
    
    //nre::gpu::copyFrameBuffer(this->framebuffer, 0, nre::gpu::FBO_DEPTHSTENCIL);
    
    nre::gpu::use_framebuffer(0);
    nre::gpu::use_wireframe = false;
    nre::gpu::set_render_mode(nre::gpu::FULLSCREEN_QUAD);
    
    nre::gpu::set_texture_slot(this->colortexture, 0);
    
    nre::gpu::draw(this->gamma_cor_prog, this->vao_fullscreen_quad);
    
    return true;
}

void NRE_Renderer::drawRenderGroup(NRE_RenderGroup &ren_group){
    
    if (!ren_group.isSetup){
       //cout << "Setting up Render group" << ren_group.camera << " " << ren_group.vgroup << " " << ren_group.mesh << endl;
        
        ren_group.isSetup = true;
        
        ren_group.vs = nre::gpu::vertex_shader();
        ren_group.vs.type = nre::gpu::VS_REGULAR;
        ren_group.vs.num_of_uvs = data_.meshes[ren_group.mesh].uvmaps;
        
        
        // choose shading mode
        ren_group.fs = nre::gpu::pixel_shader();
        lockMutex();
        switch (this->shading_mode){
            case OE_RENDERER_NORMALS_SHADING:
                ren_group.fs.type = nre::gpu::FS_NORMALS;
                break;
            case OE_RENDERER_NO_LIGHTS_SHADING:
                 ren_group.fs.type = nre::gpu::FS_MATERIAL;
                break;
            case OE_RENDERER_DIR_LIGHTS_SHADING:
                
                break;
            case OE_RENDERER_INDEXED_LIGHTS_SHADING:
                
                break;
            case OE_RENDERER_REGULAR_SHADING:
                ren_group.fs.type = nre::gpu::FS_MATERIAL;
                break;
        }
        unlockMutex();
        ren_group.fs.num_of_uvs = data_.meshes[ren_group.mesh].uvmaps;
        
        ren_group.program = nre::gpu::new_program();
        nre::gpu::set_program_vertex_shader(ren_group.program, ren_group.vs);
        nre::gpu::set_program_pixel_shader(ren_group.program, ren_group.fs);
        
        nre::gpu::setup_program(ren_group.program);
        nre::gpu::set_program_uniform_buf_slot(ren_group.program, "OE_Camera", 0);
        nre::gpu::set_program_uniform_buf_slot(ren_group.program, "OE_Mesh32", 1);
        if (nre::gpu::get_program_uniform_buf_slot(ren_group.program, "OE_Material") != -2){
            nre::gpu::set_program_uniform_buf_slot(ren_group.program, "OE_Material", 2);
        }
    }
    
    nre::gpu::set_uniform_buf_state(data_.meshes[ren_group.mesh].ubo, ren_group.program, 1, 0, 0);
    nre::gpu::set_uniform_buf_state(data_.cameras[ren_group.camera].ubo, ren_group.program, 0, 0, 0);
    nre::gpu::set_uniform_buf_state(data_.materials[ren_group.material].ubo, ren_group.program, 2, 0, 0);
    nre::gpu::draw(ren_group.program, data_.meshes[ren_group.mesh].vao, data_.vgroups[ren_group.vgroup].ibo);
}

void NRE_Renderer::drawRenderGroupZPrePass(NRE_RenderGroup& ren_group){
    
    if (!ren_group.isZPrePassSetup){
        
        ren_group.isZPrePassSetup = true;
        
        ren_group.vs_z_prepass = nre::gpu::vertex_shader();
        ren_group.vs_z_prepass.type = nre::gpu::VS_Z_PREPASS;
        ren_group.vs_z_prepass.num_of_uvs = data_.meshes[ren_group.mesh].uvmaps;
        
        ren_group.z_prepass_program = nre::gpu::new_program();
        
        nre::gpu::set_program_vertex_shader(ren_group.z_prepass_program, ren_group.vs_z_prepass);
        
        nre::gpu::setup_program(ren_group.z_prepass_program);
        nre::gpu::set_program_uniform_buf_slot(ren_group.z_prepass_program, "OE_Camera", 0);
        nre::gpu::set_program_uniform_buf_slot(ren_group.z_prepass_program, "OE_Mesh32", 1);
    }
    
    nre::gpu::set_uniform_buf_state(data_.meshes[ren_group.mesh].ubo, ren_group.z_prepass_program, 1, 0, 0);
    nre::gpu::set_uniform_buf_state(data_.cameras[ren_group.camera].ubo, ren_group.z_prepass_program, 0, 0, 0);
    nre::gpu::draw(ren_group.z_prepass_program, data_.meshes[ren_group.mesh].vao, data_.vgroups[ren_group.vgroup].ibo);
    
}

void NRE_Renderer::drawRenderGroupBoundingBox(NRE_RenderGroup& ren_group){
     
    nre::gpu::set_uniform_buf_state(data_.meshes[ren_group.mesh].ubo, this->prog_bbox, 1, 0, 0);
    nre::gpu::set_uniform_buf_state(data_.cameras[ren_group.camera].ubo, this->prog_bbox, 0, 0, 0);        
    nre::gpu::draw(this->prog_bbox, this->vao_bbox);
}

void NRE_Renderer::drawRenderGroupBoundingSphere(NRE_RenderGroup& ren_group){
    
    nre::gpu::set_uniform_buf_state(data_.meshes[ren_group.mesh].ubo, this->prog_sphere, 1, 0, 0);
    nre::gpu::set_uniform_buf_state(data_.cameras[ren_group.camera].ubo, this->prog_sphere, 0, 0, 0);        
    nre::gpu::draw(this->prog_sphere, this->vao_sphere, this->ibo_sphere);
}


void NRE_Renderer::setupBoundingBoxProgram(){
    
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
        
    nre::gpu::vertex_shader vs_bbox;
    nre::gpu::pixel_shader fs_bbox;
        
    vs_bbox.type = nre::gpu::VS_BOUNDING_BOX;
    vs_bbox.num_of_uvs = 0;
        
    fs_bbox.type = nre::gpu::FS_NORMALS;
    fs_bbox.num_of_uvs = 0;
        
    nre::gpu::set_program_vertex_shader(this->prog_bbox, vs_bbox);
    nre::gpu::set_program_pixel_shader(this->prog_bbox, fs_bbox);
        
    nre::gpu::setup_program(this->prog_bbox);
    nre::gpu::set_program_uniform_buf_slot(this->prog_bbox, "OE_Camera", 0);
    nre::gpu::set_program_uniform_buf_slot(this->prog_bbox, "OE_Mesh32", 1);
    if (nre::gpu::get_program_uniform_buf_slot(this->prog_bbox, "OE_Material") != -2){
        nre::gpu::set_program_uniform_buf_slot(this->prog_bbox, "OE_Material", 2);
    }
}

void NRE_Renderer::setupBoundingSphereProgram(){
    
    this->prog_sphere = nre::gpu::new_program();
        
    nre::gpu::vertex_shader vs_sphere;
    nre::gpu::pixel_shader fs_sphere;
        
    vs_sphere.type = nre::gpu::VS_BOUNDING_SPHERE;
    vs_sphere.num_of_uvs = 0;
        
    fs_sphere.type = nre::gpu::FS_NORMALS;
    fs_sphere.num_of_uvs = 0;
        
    nre::gpu::set_program_vertex_shader(this->prog_sphere, vs_sphere);
    nre::gpu::set_program_pixel_shader(this->prog_sphere, fs_sphere);
        
    nre::gpu::setup_program(this->prog_sphere);
    nre::gpu::set_program_uniform_buf_slot(this->prog_sphere, "OE_Camera", 0);
    nre::gpu::set_program_uniform_buf_slot(this->prog_sphere, "OE_Mesh32", 1);
    if (nre::gpu::get_program_uniform_buf_slot(this->prog_sphere, "OE_Material") != -2){
        nre::gpu::set_program_uniform_buf_slot(this->prog_sphere, "OE_Material", 2);
    }
    
}

//---------------------------------Update actual GPU data------------------------------//

void NRE_Renderer::updateMeshGPUData(){
    for (auto mesh: data_.meshes){
        
        // first time buffer initialization
        if (!data_.meshes[mesh.first].has_init){
            data_.meshes[mesh.first].vbo      = nre::gpu::new_vertex_buf();
            data_.meshes[mesh.first].vao      = nre::gpu::new_vertex_layout();
            data_.meshes[mesh.first].ubo      = nre::gpu::new_uniform_buf();
            data_.meshes[mesh.first].has_init = true;
        }
        
        
        // initialize data itself
        if (!data_.meshes[mesh.first].vao_initialized){
            
            /// vertex buffer
            data_.meshes[mesh.first].mesh->data->vbo_mutex.lockMutex();
            nre::gpu::set_vertex_buf_memory_and_data(data_.meshes[mesh.first].vbo, data_.meshes[mesh.first].mesh->data->vbo, nre::gpu::STATIC);
            data_.meshes[mesh.first].mesh->data->vbo.clear();
            data_.meshes[mesh.first].mesh->data->vbo_mutex.unlockMutex();
            
            
            /// index buffers
            data_.meshes[mesh.first].mesh->data->ibos_mutex.lockMutex();
            for (auto vg : mesh.second.vgroups){
                data_.vgroups[vg].ibo = nre::gpu::new_index_buf();
                nre::gpu::set_index_buf_memory_and_data(data_.vgroups[vg].ibo, data_.meshes[mesh.first].mesh->data->ibos[vg].data, nre::gpu::STATIC);
                data_.meshes[mesh.first].mesh->data->ibos[vg].data.clear();
            }
            
            data_.meshes[mesh.first].mesh->data->ibos_mutex.unlockMutex();
            
            /// vertex layout
            typedef nre::gpu::vertex_layout_input VLI; // for clarity
            
            data_.meshes[mesh.first].mesh->data->ibos.clear();
            //delete data_.meshes[mesh.first].mesh->data->index_buffer;
            //data_.meshes[mesh.first].mesh->data->index_buffer = nullptr;
            
            data_.meshes[mesh.first].vao_input.clear();
        
            data_.meshes[mesh.first].vao_input.push_back(VLI(data_.meshes[mesh.first].vbo, 0, 3, 6+data_.meshes[mesh.first].uvmaps*2));
            data_.meshes[mesh.first].vao_input.push_back(VLI(data_.meshes[mesh.first].vbo, 3, 3, 6+data_.meshes[mesh.first].uvmaps*2));
        
            for (size_t i=0; i < data_.meshes[mesh.first].uvmaps; i++){
                data_.meshes[mesh.first].vao_input.push_back(VLI(data_.meshes[mesh.first].vbo, 6+2*i, 2, 6+data_.meshes[mesh.first].uvmaps*2));
            }
            
            nre::gpu::set_vertex_layout_format(data_.meshes[mesh.first].vao, data_.meshes[mesh.first].vao_input);
            data_.meshes[mesh.first].vao_initialized = true;

        }
        
        // update per frame data
        if (data_.meshes[mesh.first].changed){
            
            
            data_.meshes[mesh.first].data = OE_Mat4x4ToSTDVector(data_.meshes[mesh.first].model_mat);
            
            // populate scaling_max_data
            data_.meshes[mesh.first].data.push_back(data_.meshes[mesh.first].max_x);
            data_.meshes[mesh.first].data.push_back(data_.meshes[mesh.first].max_y);
            data_.meshes[mesh.first].data.push_back(data_.meshes[mesh.first].max_z);
            
            // final element is sphere radius candidate            
            data_.meshes[mesh.first].data.push_back(data_.meshes[mesh.first].max_radius);
            
            // populate scaling_min_data
            data_.meshes[mesh.first].data.push_back(data_.meshes[mesh.first].min_x);
            data_.meshes[mesh.first].data.push_back(data_.meshes[mesh.first].min_y);
            data_.meshes[mesh.first].data.push_back(data_.meshes[mesh.first].min_z);
            
            // final element is sphere radius candidate            
            data_.meshes[mesh.first].data.push_back(data_.meshes[mesh.first].min_radius);
            
            
            if (data_.meshes[mesh.first].size != data_.meshes[mesh.first].data.size()){
                data_.meshes[mesh.first].size = data_.meshes[mesh.first].data.size();
                nre::gpu::set_uniform_buf_memory(data_.meshes[mesh.first].ubo, data_.meshes[mesh.first].size, nre::gpu::DYNAMIC);
            }
            
            nre::gpu::set_uniform_buf_data(data_.meshes[mesh.first].ubo, data_.meshes[mesh.first].data, 0);
            
            data_.meshes[mesh.first].changed = false;
        }
    }
    
    for (auto cam: data_.deleted_meshes){
        this->sce_ren_groups[data_.meshes[cam].scene_id].removeMesh(cam);
        data_.deleteMesh(cam);
    }
    data_.deleted_meshes.clear();    
}

void NRE_Renderer::updateMaterialGPUData(){
    for (auto mat: data_.materials){
        
        if (!data_.materials[mat.first].has_init){
            data_.materials[mat.first].ubo = nre::gpu::new_uniform_buf();
            data_.materials[mat.first].has_init = true;
        }
        
        if (data_.materials[mat.first].changed){
            
            if (data_.materials[mat.first].size != data_.materials[mat.first].data.size()){
                data_.materials[mat.first].size = data_.materials[mat.first].data.size();
                nre::gpu::set_uniform_buf_memory(data_.materials[mat.first].ubo, data_.materials[mat.first].size, nre::gpu::DYNAMIC);
            }
            
            nre::gpu::set_uniform_buf_data(data_.materials[mat.first].ubo, data_.materials[mat.first].data, 0);
            data_.materials[mat.first].changed = false;
        }
    }
    
    for (auto cam: data_.deleted_materials){
        this->sce_ren_groups[data_.materials[cam].scene_id].removeMaterial(cam);
        data_.deleteMaterial(cam);
    }
    data_.deleted_materials.clear();
}

void NRE_Renderer::updateCameraGPUData(){
    for (auto cam: data_.cameras){
        
        if (!data_.cameras[cam.first].has_init){
            data_.cameras[cam.first].ubo = nre::gpu::new_uniform_buf();
            data_.cameras[cam.first].has_init = true;
        }
        if (data_.cameras[cam.first].changed){
            
            if (data_.cameras[cam.first].size != data_.cameras[cam.first].data.size()){
                data_.cameras[cam.first].size = data_.cameras[cam.first].data.size();
                nre::gpu::set_uniform_buf_memory(data_.cameras[cam.first].ubo, data_.cameras[cam.first].size, nre::gpu::DYNAMIC);
            }
            
            nre::gpu::set_uniform_buf_data(data_.cameras[cam.first].ubo, data_.cameras[cam.first].data, 0);
            data_.cameras[cam.first].changed = false;
        }
    }
    
    for (auto cam: data_.deleted_cameras){
        this->sce_ren_groups[data_.cameras[cam].scene_id].removeCamera(cam);
        data_.deleteCamera(cam);
    }
    data_.deleted_cameras.clear();
}
void NRE_Renderer::updateLightGPUData(){
    /*for (auto l: data_.pt_lights){
        
        if (!data_.pt_lights[l.first].has_init){
            data_.pt_lights[l.first].ubo = nre::gpu::new_uniform_buf();
            data_.pt_lights[l.first].has_init = true;
        }
        if (data_.pt_lights[l.first].changed){
            
            if (data_.pt_lights[l.first].size != data_.pt_lights[l.first].data.size()){
                data_.pt_lights[l.first].size = data_.pt_lights[l.first].data.size();
                nre::gpu::set_uniform_buf_memory(data_.pt_lights[l.first].ubo, data_.pt_lights[l.first].size, nre::gpu::DYNAMIC);
            }
            
            nre::gpu::set_uniform_buf_data(data_.pt_lights[l.first].ubo, data_.pt_lights[l.first].data, 0);
            data_.pt_lights[l.first].changed = false;
        }
    }*/
    std::vector<float> pt_light_data;
    pt_light_data.reserve(16*255);
    int count = 0;
    for (auto l: this->pt_visible_lights){
        pt_light_data.insert(pt_light_data.end(), data_.pt_lights[l.id].data.begin(), data_.pt_lights[l.id].data.end());
        
        count++;
        if (count > 255)
            break;
    }
    
    // fill the rest with zeros since webgl complains otherwise
    for (int i=0; i< 16*(255-count); i++){
        pt_light_data.push_back(0.0f);
    }
    nre::gpu::set_uniform_buf_memory(this->pt_light_ubo, pt_light_data.size(), nre::gpu::STREAM);
    nre::gpu::set_uniform_buf_data(this->pt_light_ubo, pt_light_data, 0);
}

bool NRE_Renderer::updateMultiThread(OE_Task*, int){
    return false;
}

void NRE_Renderer::destroy(){
    
    this->setup_bbox_prog = false;
    this->setup_sphere_prog = false;
    
    data_.clear();
    this->sce_ren_groups.clear();
    
    nre::gpu::destroy();
}    

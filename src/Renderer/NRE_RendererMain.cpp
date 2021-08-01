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
    /*cout << "NRE Cameras: " << this->cameras.size() << endl;
    cout << "NRE Materials: " << this->materials.size() << endl;
    cout << "NRE Meshes: " << this->meshes.size() << endl;
    cout << "NRE Vgroups: " << this->vgroups.size() << endl;*/
    
    // make sure we use the right API
    if (this->screen->restart_renderer)
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
    
    /*cout << "NRE Cameras: " << this->cameras.size() << endl;
    cout << "NRE Materials: " << this->materials.size() << endl;
    cout << "NRE Meshes: " << this->meshes.size() << endl;
    cout << "NRE Vgroups: " << this->vgroups.size() << endl;*/
    
    nre::gpu::use_framebuffer(this->framebuffer);
    nre::gpu::clear_framebuffer(this->framebuffer, nre::gpu::FBO_ALL, 1.0f);

    if (this->loaded_viewport != 0){

        auto lv = this->loaded_viewport;
        
        if (this->viewports[lv].cameras.size() == 0){
            throw nre::incomplete_viewport(lv);
        }
        // TEMPORARY until multiple layers and cameras support is implemented
        else if (this->viewports[lv].cameras.size() > 1){
            throw nre::unsupported_viewport(lv, "Too many (" + to_string(this->viewports[lv].cameras.size()) + ") cameras");
        }
        else{
            
        }
        
        // only one fullscreen camera is supported for now
        auto camera_id = this->viewports[lv].cameras[0];
        auto scene_id = this->cameras[camera_id].scene_id;
        
        // draw everything required for the z prepass, which also populates the depth buffer
        nre::gpu::set_render_mode(nre::gpu::Z_PREPASS_BACKFACE);
        for (auto x: this->scenes[scene_id].render_groups){
            if (x.camera == camera_id){
                this->drawRenderGroupZPrePass(x);
                this->scenes[scene_id].render_groups.replace(x);
            }
        }
        this->scenes[scene_id].render_groups.update();
        
        //sort point lights
        this->sortPointLights(scene_id, camera_id);
        this->updateLightGPUData();
        
        nre::gpu::use_framebuffer(this->fbo_light);
        nre::gpu::clear_framebuffer(this->fbo_light, nre::gpu::FBO_COLORSTENCIL, 0.0f);
        
        nre::gpu::set_render_mode(nre::gpu::LIGHT_PREPASS_2);
        
        nre::gpu::set_uniform_buf_state(this->pt_light_ubo, this->prog_light, 1, 0, 0);
        nre::gpu::set_uniform_buf_state(this->cameras[camera_id].ubo, this->prog_light, 0, 0, 0);    
        
        int lights_count = this->pt_visible_lights.size();
        
        nre::gpu::draw_instanced(this->prog_light, this->vao_sphere, this->ibo_sphere, std::min<int>(lights_count, 255));
        
        nre::gpu::set_render_mode(nre::gpu::LIGHT_AFTERPASS_RG);
        nre::gpu::draw_instanced(this->prog_light, this->vao_sphere, this->ibo_sphere, std::min<int>(lights_count, 255));
        
        nre::gpu::set_render_mode(nre::gpu::LIGHT_AFTERPASS_BA);
        nre::gpu::draw_instanced(this->prog_light, this->vao_sphere, this->ibo_sphere, std::min<int>(lights_count, 255));
        
        // draw everything normally
        nre::gpu::use_framebuffer(this->framebuffer);
        nre::gpu::set_render_mode(nre::gpu::AFTERPREPASS_BACKFACE);
        for (auto x: this->scenes[scene_id].render_groups){
            if (x.camera == camera_id){
                this->drawRenderGroup(x);
                this->scenes[scene_id].render_groups.replace(x);
            }
        }
        this->scenes[scene_id].render_groups.update();
    
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
            for (auto x: this->scenes[scene_id].render_groups){
                if (x.camera == camera_id){
                    this->drawRenderGroupBoundingBox(x);
                    this->scenes[scene_id].render_groups.replace(x);
                }
            }
            this->scenes[scene_id].render_groups.update();
        }
        if (render_spheres){
            nre::gpu::set_render_mode(nre::gpu::REGULAR_BOTH);
            if (!this->setup_sphere_prog){
        
                this->setupBoundingSphereProgram();
                this->setup_sphere_prog = true;
            }
            for (auto x: this->scenes[scene_id].render_groups){
                if (x.camera == camera_id){
                    this->drawRenderGroupBoundingSphere(x);
                    this->scenes[scene_id].render_groups.replace(x);
                }
            }
            this->scenes[scene_id].render_groups.update();
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

void NRE_Renderer::sortPointLights(std::size_t scene_id, std::size_t camera_id){
    this->pt_visible_lights.clear();
    
    auto persp_mat = this->cameras[camera_id].perspective_mat;
    
    // set limit to 255 lights
    // why you ask?
    // Well firstly 255 lights is already a LOT of lights inside the view frustum. More lights are not that good for performance
    // Secondly the minimum required OpenGL Uniform buffer limit can only contain 256 matrices,
    // so i can only place up to 255 lights (light with index 0 inside the shaders symbolizes the lack of lights)
    // Thirdly, spot lights will be handed in a separate pass with another texture
    
    for (auto l: this->scenes[scene_id].pt_lights){
        
        // calculations happening in view space
        OE_Vec4 model_pos = OE_Vec4(this->pt_lights[l].model_mat[3].x, this->pt_lights[l].model_mat[3].y, this->pt_lights[l].model_mat[3].z, 1.0f);
        
        OE_Vec4 light_view_pos = this->cameras[camera_id].view_mat*model_pos;
            
        auto light_camview_radius = light_view_pos - OE_Vec4(this->pt_lights[l].range, this->pt_lights[l].range, this->pt_lights[l].range, 0.0f);
        
        bool is_behind_camera = light_camview_radius.z > (-this->cameras[camera_id].near);
        bool is_too_far_away = (light_camview_radius.z + 2*this->pt_lights[l].range) < (-this->cameras[camera_id].far);
        
        //calculations happening in clip space        
        auto light_clip_radius_max_x = OE_GetClipCoords(persp_mat, light_view_pos + OE_Vec4(this->pt_lights[l].range, 0.0f, 0.0f, 0.0f));
        
        auto light_clip_radius_min_x = OE_GetClipCoords(persp_mat, light_view_pos - OE_Vec4(this->pt_lights[l].range, 0.0f, 0.0f, 0.0f));
        
        auto light_clip_radius_max_y = OE_GetClipCoords(persp_mat, light_view_pos + OE_Vec4(0.0f, this->pt_lights[l].range, 0.0f, 0.0f));
        
        auto light_clip_radius_min_y = OE_GetClipCoords(persp_mat, light_view_pos - OE_Vec4(0.0f, this->pt_lights[l].range, 0.0f, 0.0f));
        
        bool is_too_left  = light_clip_radius_max_x.x < -1.02f;
        bool is_too_right = light_clip_radius_min_x.x > 1.02f;

        bool is_too_above = light_clip_radius_max_y.y < -1.02f;
        bool is_too_below = light_clip_radius_min_y.y > 1.02f;
        
        // only choose lights that have effects inside the view frustum.
        // This means that up to 254 lights in range of the camera will be active at once
        // This means the scene can have many many more lights overall. All that matters
        // is what's VISIBLE
        if ((not is_behind_camera) and (not is_too_far_away) and (not is_too_left) and (not is_too_right) and (not is_too_above) and (not is_too_below)){
        auto point_light = NRE_PointLightDrawCall(l, (this->cameras[camera_id].perspective_view_mat*this->pt_lights[l].model_mat[3])[2], 0);
        this->pt_visible_lights.insert(point_light);
        
        }
        
    }
    //cout << "pt size " << this->pt_visible_lights.size() << endl;
}

void NRE_Renderer::drawRenderGroup(NRE_RenderGroup &ren_group){
    
    if (!ren_group.isSetup){
       //cout << "Setting up Render group" << ren_group.camera << " " << ren_group.vgroup << " " << ren_group.mesh << endl;
        
        ren_group.isSetup = true;
        
        ren_group.vs = nre::gpu::vertex_shader();
        ren_group.vs.type = nre::gpu::VS_REGULAR;
        ren_group.vs.num_of_uvs = this->meshes[ren_group.mesh].uvmaps;
        
        
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
        ren_group.fs.num_of_uvs = this->meshes[ren_group.mesh].uvmaps;
        
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
    
    nre::gpu::set_uniform_buf_state(this->meshes[ren_group.mesh].ubo, ren_group.program, 1, 0, 0);
    nre::gpu::set_uniform_buf_state(this->cameras[ren_group.camera].ubo, ren_group.program, 0, 0, 0);
    nre::gpu::set_uniform_buf_state(this->materials[ren_group.material].ubo, ren_group.program, 2, 0, 0);
    nre::gpu::draw(ren_group.program, this->meshes[ren_group.mesh].vao, this->vgroups[ren_group.vgroup].ibo);
}

void NRE_Renderer::drawRenderGroupZPrePass(NRE_RenderGroup& ren_group){
    
    if (!ren_group.isZPrePassSetup){
        
        ren_group.isZPrePassSetup = true;
        
        ren_group.vs_z_prepass = nre::gpu::vertex_shader();
        ren_group.vs_z_prepass.type = nre::gpu::VS_Z_PREPASS;
        ren_group.vs_z_prepass.num_of_uvs = this->meshes[ren_group.mesh].uvmaps;
        
        ren_group.z_prepass_program = nre::gpu::new_program();
        
        nre::gpu::set_program_vertex_shader(ren_group.z_prepass_program, ren_group.vs_z_prepass);
        
        nre::gpu::setup_program(ren_group.z_prepass_program);
        nre::gpu::set_program_uniform_buf_slot(ren_group.z_prepass_program, "OE_Camera", 0);
        nre::gpu::set_program_uniform_buf_slot(ren_group.z_prepass_program, "OE_Mesh32", 1);
    }
    
    nre::gpu::set_uniform_buf_state(this->meshes[ren_group.mesh].ubo, ren_group.z_prepass_program, 1, 0, 0);
    nre::gpu::set_uniform_buf_state(this->cameras[ren_group.camera].ubo, ren_group.z_prepass_program, 0, 0, 0);
    nre::gpu::draw(ren_group.z_prepass_program, this->meshes[ren_group.mesh].vao, this->vgroups[ren_group.vgroup].ibo);
    
}

void NRE_Renderer::drawRenderGroupBoundingBox(NRE_RenderGroup& ren_group){
     
    nre::gpu::set_uniform_buf_state(this->meshes[ren_group.mesh].ubo, this->prog_bbox, 1, 0, 0);
    nre::gpu::set_uniform_buf_state(this->cameras[ren_group.camera].ubo, this->prog_bbox, 0, 0, 0);        
    nre::gpu::draw(this->prog_bbox, this->vao_bbox);
}

void NRE_Renderer::drawRenderGroupBoundingSphere(NRE_RenderGroup& ren_group){
    
    nre::gpu::set_uniform_buf_state(this->meshes[ren_group.mesh].ubo, this->prog_sphere, 1, 0, 0);
    nre::gpu::set_uniform_buf_state(this->cameras[ren_group.camera].ubo, this->prog_sphere, 0, 0, 0);        
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

void NRE_Renderer::generateDrawCalls(){
    
    // delete all remaining draw call programs from deleted scenes
    for (auto &sce : deleted_scenes){
        for (auto ren_group : this->scenes[sce].render_groups){
        
            if (ren_group.z_prepass_program != 0)
                nre::gpu::del_program(ren_group.z_prepass_program);
            if (ren_group.program != 0)
                nre::gpu::del_program(ren_group.program);
        }
        this->scenes.erase(sce);
    }
    this->deleted_scenes.clear();
    
    for (auto &scene: scenes){
        
        // delete remaining draw call programs from existing scenes
        for (auto ren_group : scene.second.render_groups.to_be_deleted_){
        
            if (ren_group.z_prepass_program != 0)
                nre::gpu::del_program(ren_group.z_prepass_program);
            if (ren_group.program != 0)
                nre::gpu::del_program(ren_group.program);
        }
        scene.second.render_groups.cleanupPrograms();
        
        // gemerate draw calls anew
        for (auto cam : scene.second.cameras){
            for (auto mesh : scene.second.meshes){
                for (auto vgroup : this->meshes[mesh].vgroups){
                    auto render_data = NRE_RenderGroup();
                    render_data.camera = cam;
                    render_data.vgroup = vgroup;
                    render_data.mesh = mesh;
                    render_data.material = this->vgroups[vgroup].material_id;
                    if (!scene.second.existsRenderGroup(render_data)){
                        scene.second.render_groups.insert(render_data);
                    }
                }
            }
        }
    }
}


bool NRE_Renderer::updateMultiThread(OE_Task*, int){
    return false;
}

void NRE_Renderer::destroy(){
    
    this->cameras.clear();
    this->materials.clear();
    this->vgroups.clear();
    this->meshes.clear();
    this->dir_lights.clear();
    this->pt_lights.clear();
    this->scenes.clear();
    this->viewports.clear();
    
    this->setup_bbox_prog = false;
    this->setup_sphere_prog = false;
    
    nre::gpu::destroy();
}    

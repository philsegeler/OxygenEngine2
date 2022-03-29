#include <Renderer/DataHandler/data_handler.h>
#include <api_oe.h>

using namespace oe;
using namespace std;

bool NRE_DataHandler::update(bool restart_renderer, bool load_minmax_elements){
    
    this->load_spheres_or_bboxes = load_minmax_elements;
    // add/change any missing world data
    //OE_Main->lockMutex();
    
    vector<size_t> camera_ids;
    
    // this regenerates the renderer database
    
    if (restart_renderer){
        
        //cout << OE_Main->world->to_str() << endl;
        
        for (auto mat : OE_World::materialsList){
            mat.flag_as_changed();
        }
        
        for (auto obj : OE_World::objectsList){
            if (obj.p_->getType() == OE_OBJECT_MESH){
                auto mesh = static_pointer_cast<OE_Mesh32>(obj.p_);
                mesh->data->vbo_exists = false;
                mesh->data->ibos_exist = false;
            }
            obj.flag_as_changed();
        }
        
        for (auto sce : OE_World::scenesList){
            sce.flag_as_changed();
        }
        
        for (auto vpc : OE_World::viewportsList){
            vpc.flag_as_changed();
        }
    }
    
    // Update element data
    
    for (auto mat : OE_World::materialsList.changed()){
        this->handleMaterialData(mat.id_, mat.p_);
    }
    
    this->has_dir_lights_changed = false;
    this->has_pt_lights_changed = false;
    
    for (auto obj : OE_World::objectsList.changed()){
        if (obj.p_->getType() == OE_OBJECT_MESH){
           this->handleMeshData(obj.id_, static_pointer_cast<OE_Mesh32>(obj.p_));
        }
        else if (obj.p_->getType() == OE_OBJECT_LIGHT){
            this->handleLightData(obj.id_, static_pointer_cast<OE_Light>(obj.p_));
        }
    }
    
    for (auto obj : OE_World::objectsList.changed()){
        if (obj.p_->getType() == OE_OBJECT_CAMERA){
           this->handleCameraData(obj.id_, static_pointer_cast<OE_Camera>(obj.p_));
           camera_ids.push_back(obj.id_);
        }
    }
    
    for (auto sce : OE_World::scenesList.changed()){
        this->handleSceneData(sce.id_, sce.p_);
    }
    
    for (auto vpc : OE_World::viewportsList.changed()){
        this->handleViewportData(vpc.id_, vpc.p_);
        this->loaded_viewport = OE_Main->world->loaded_viewport;
    }
    
    // handle deleted elements (should look if the element exists first)
    
    for (auto mat : OE_World::materialsList.deleted()){
        if (this->materials.count(mat.id_) == 1){
            this->deleted_materials.insert(mat.id_);
        }
        
    }

    for (auto obj : OE_World::objectsList.deleted()){
        
        if (this->meshes.count(obj.id_) == 1){
            this->deleted_meshes.insert(obj.id_);
        }
        else if (this->dir_lights.count(obj.id_) == 1){
            this->dir_lights.erase(obj.id_);
            this->has_dir_lights_changed = true;
        }
        else if (this->pt_lights.count(obj.id_) == 1){
            this->pt_lights.erase(obj.id_);
            this->has_pt_lights_changed = true;
        }
        else if (this->cameras.count(obj.id_) == 1){
            this->deleted_cameras.insert(obj.id_);
        }
        else {
            
        }
    }

    
    for (auto sce : OE_World::scenesList.deleted()){
        if (this->scenes.count(sce.id_) == 1){
            this->deleted_scenes.insert(sce.id_);
        }
    }
    
    for (auto vpc : OE_World::viewportsList.deleted()){
        if (this->viewports.count(vpc.id_) == 1){
            this->viewports.erase(vpc.id_);
            if (this->loaded_viewport == vpc.id_){
                this->loaded_viewport = 0; // needed for compatibility with older .csl with no viewportconfigs
            }
        }
    }
    
    //OE_Main->unlockMutex();
    return true;
}

//-----------------------------------Pass individual element changes synchronously as fast as possible--------//

void NRE_DataHandler::handleMeshData(std::size_t id, std::shared_ptr<OE_Mesh32> mesh){
    if (this->meshes.count(id) == 0){
        
        // make sure all index and vertex buffers are generated
        // On loading objects from disk they are not generated here
        // But on objects created on the fly they ARE generated here
        mesh->data->genVertexBufferInternally();
        mesh->data->genIndexBuffersInternally();
        
        // setup the Vertex Buffer
        this->meshes[id] = NRE_MeshRenderData(); this->meshes[id].id = id;
        this->meshes[id].uvmaps = mesh->data->vertices.uvmaps.size();
        this->meshes[id].size = mesh->data->vbo.size();
        
        // store the shared pointer
        this->meshes[id].mesh = mesh;        
        
        //setup the Uniform buffer holding the model matrix
        // but offload the actual OpenGL commands for later, since this runs in a performance
        // critical section
        
        this->meshes[id].model_mat = mesh->GetModelMatrix();
        
        //this->meshes[id].data = OE_Mat4x4ToSTDVector(model_mat);
        this->meshes[id].changed = true;
        
        // handle Vertex (Triangle) groups
        for (auto vgroup : mesh->data->triangle_groups){
            this->handleVGroupData(id, vgroup.first, mesh);
            this->meshes[id].vgroups.insert(vgroup.first);
        }
        
        // store bounding box
        
        //---- <TEMPORARY> ----//
        // This should not be done here but in the physics engine or interpreter
        mesh->calculateProperBoundingBox();
        //---- </TEMPORARY> ---//
        
        assert (mesh->data->vertices.calculatedBoundingBox);
        
        this->meshes[id].max_x = mesh->data->vertices.max_x;
        this->meshes[id].max_y = mesh->data->vertices.max_y;
        this->meshes[id].max_z = mesh->data->vertices.max_z;
        
        this->meshes[id].min_x = mesh->data->vertices.min_x;
        this->meshes[id].min_y = mesh->data->vertices.min_y;
        this->meshes[id].min_z = mesh->data->vertices.min_z;
        
        this->meshes[id].max_radius = mesh->data->vertices.max_radius;
        this->meshes[id].min_radius = mesh->data->vertices.min_radius;
    }
    else{
        this->meshes[id].model_mat = mesh->GetModelMatrix();
        
        if (this->load_spheres_or_bboxes){
            //---- <TEMPORARY> ----//
            // This should not be done here but in the physics engine
            mesh->calculateProperBoundingBox();
            //---- </TEMPORARY> ---//
            
            assert (mesh->data->vertices.calculatedBoundingBox);
        
            this->meshes[id].max_x = mesh->data->vertices.max_x;
            this->meshes[id].max_y = mesh->data->vertices.max_y;
            this->meshes[id].max_z = mesh->data->vertices.max_z;
        
            this->meshes[id].min_x = mesh->data->vertices.min_x;
            this->meshes[id].min_y = mesh->data->vertices.min_y;
            this->meshes[id].min_z = mesh->data->vertices.min_z;
            
            this->meshes[id].max_radius = mesh->data->vertices.max_radius;
            this->meshes[id].min_radius = mesh->data->vertices.min_radius;
            
        }
        
        this->meshes[id].changed = true;
    }
}

void NRE_DataHandler::handleVGroupData(std::size_t mesh_id, std::size_t id, std::shared_ptr<OE_Mesh32> mesh){
    if (this->vgroups.count(id) == 0){
        
        this->vgroups[id] = NRE_VGroupRenderData(); this->vgroups[id].id = id; this->vgroups[id].mesh_id = mesh_id;
        this->vgroups[id].bone_mat = OE_Mat4x4(1.0f);
        this->vgroups[id].ibo = 0;
        this->vgroups[id].material_id = mesh->data->triangle_groups[id]->material_id;        
    }
}

void NRE_DataHandler::handleMaterialData(std::size_t id, std::shared_ptr<OE_Material> mat){
    if (this->materials.count(id) == 0){
        this->materials[id] = NRE_MaterialRenderData(); this->materials[id].id = id;
    }
    
    this->materials[id].data = mat->GetRendererData();
    this->materials[id].changed = true;
}

void NRE_DataHandler::handleCameraData(std::size_t id, std::shared_ptr<OE_Camera> camera){
    if (this->cameras.count(id) == 0){
        
        //setup the Uniform buffer holding the perspective/view matrix
        // but offload the actual OpenGL commands for later, since this runs in a performance
        // critical section
        this->cameras[id] = NRE_CameraRenderData(); this->cameras[id].id = id;
    }
    
    this->cameras[id].near = (float)camera->near+0.3f;
    this->cameras[id].far = (float)camera->far + 50.0f;
    
    auto view_mat = camera->GetViewMatrix();
    auto perspective_mat = OE_Perspective(camera->fov, camera->aspect_ratio, this->cameras[id].near, this->cameras[id].far);
    
    this->cameras[id].view_mat = view_mat;
    this->cameras[id].perspective_mat = perspective_mat;
    this->cameras[id].model_mat = camera->GetModelMatrix();
    this->cameras[id].perspective_view_mat = perspective_mat*view_mat;
    
    this->cameras[id].data = OE_Mat4x4ToSTDVector(this->cameras[id].perspective_view_mat);

    this->cameras[id].data.push_back(this->cameras[id].model_mat[3].x);
    this->cameras[id].data.push_back(this->cameras[id].model_mat[3].y);
    this->cameras[id].data.push_back(this->cameras[id].model_mat[3].z);
    this->cameras[id].data.push_back(1.0f);
    this->cameras[id].changed = true;
}

void NRE_DataHandler::handleLightData(std::size_t id, std::shared_ptr<OE_Light> light){
    if (light->light_type == 1){
        //POINT LIGHT
        if (this->pt_lights.count(id) == 0){
            this->pt_lights[id] = NRE_PointLightRenderData(); this->pt_lights[id].id = id;
            this->has_pt_lights_changed = true;
        }
        
        this->pt_lights[id].model_mat = light->GetModelMatrix();
        this->pt_lights[id].color     = light->color;
        this->pt_lights[id].intensity = light->intensity;
        this->pt_lights[id].range     = light->range/20.0f;
            
        //this->pt_lights[id].data = light->GetLightGPUData();
        this->pt_lights[id].data = OE_Mat4x4ToSTDVector(this->pt_lights[id].model_mat);
        /*this->pt_lights[id].data.push_back(1.0f);
        this->pt_lights[id].data.push_back(1.0f);
        this->pt_lights[id].data.push_back(1.0f);         
        this->pt_lights[id].data.push_back(light->range/20.0f);
        this->pt_lights[id].data.push_back(1.0f);
        this->pt_lights[id].data.push_back(1.0f);
        this->pt_lights[id].data.push_back(1.0f);       
        this->pt_lights[id].data.push_back(light->range/20.0f);*/
        this->pt_lights[id].data[15] = light->range/20.0f;
            
        this->pt_lights[id].changed = true;
    }
    else if (light->light_type == 2){
        //DIRECTIONAL LIGHT
    }
    else if (light->light_type == 3){
        // SPOTLIGHT
    }
    else{
        //NOTHING
    }
}

void NRE_DataHandler::handleSceneData(std::size_t id, std::shared_ptr<OE_Scene> scene){
    if (this->scenes.count(id) == 0){
        this->scenes[id] = NRE_SceneRenderData(); this->scenes[id].id = id;
    }
    else{
        this->scenes[id].cameras.clear();
        this->scenes[id].dir_lights.clear();
        this->scenes[id].pt_lights.clear();
        this->scenes[id].meshes.clear();
    }
    
    // group objects
    for (auto x: scene->objects){
        if (this->cameras.count(x) != 0){
            this->scenes[id].cameras.insert(x);
        }
        else if (this->meshes.count(x) != 0){
            this->scenes[id].meshes.insert(x);
        }
        else if (this->dir_lights.count(x) != 0){
            this->scenes[id].dir_lights.insert(x);
        }
        else if (this->pt_lights.count(x) != 0){
            this->scenes[id].pt_lights.insert(x);
        }
    }
        
    // store the scene each camera is in
    for (auto cam : this->scenes[id].cameras){
        this->cameras[cam].scene_id = id;
        this->cameras[cam].changed = true;
    }
    // store the scene each mesh is in
    for (auto mat : this->scenes[id].materials){
        this->materials[mat].scene_id = id;
        this->materials[mat].changed = true;
    }
    // store the scene each material is in
    for (auto mesh : this->scenes[id].meshes){
        this->meshes[mesh].scene_id = id;
        this->meshes[mesh].changed = true;
    }
    
    this->scenes[id].materials = scene->materials;
    this->scenes[id].changed = true;
}

void NRE_DataHandler::handleViewportData(std::size_t id, std::shared_ptr<OE_ViewportConfig> vp_config){
    if (this->viewports.count(id) == 0){
        this->viewports[id] = NRE_ViewportRenderData(); this->viewports[id].id = id;
        this->viewports[id].has_init = true;
    }
    
    this->viewports[id].layers = vp_config->layers;
    this->viewports[id].cameras = vp_config->cameras;
    this->viewports[id].camera_modes = vp_config->camera_modes;
    this->viewports[id].layer_combine_modes = vp_config->layer_combine_modes;
    this->viewports[id].split_screen_positions = vp_config->split_screen_positions;
    this->viewports[id].changed = true;
}

// deletes

void NRE_DataHandler::deleteCamera(std::size_t id){
    
    if (this->cameras[id].ubo != 0){
        nre::gpu::del_uniform_buf(this->cameras[id].ubo);
    }
    
    this->cameras.erase(id);
}

void NRE_DataHandler::deleteMaterial(std::size_t id){
    
    if (this->materials[id].ubo != 0){
        nre::gpu::del_uniform_buf(this->materials[id].ubo);
    }
    
    this->materials.erase(id);
}

void NRE_DataHandler::deleteMesh(std::size_t id){
    
    // delete all buffers
    if (this->meshes[id].vbo !=0){
        nre::gpu::del_vertex_buf(this->meshes[id].vbo);
    }
    if (this->meshes[id].vao !=0){
        nre::gpu::del_vertex_layout(this->meshes[id].vao);
    }
    if (this->meshes[id].ubo !=0){
        nre::gpu::del_uniform_buf(this->meshes[id].ubo);
    }
    
    // delete buffers of vertex groups
    for (auto vgroup : this->meshes[id].vgroups){
        if (this->vgroups[vgroup].ibo != 0){
            nre::gpu::del_index_buf(this->vgroups[vgroup].ibo);
        }
        this->vgroups.erase(vgroup);
    }    
    this->meshes.erase(id);
}

void NRE_DataHandler::clear(){
    
    this->cameras.clear();
    this->materials.clear();
    this->vgroups.clear();
    this->meshes.clear();
    this->dir_lights.clear();
    this->pt_lights.clear();
    this->scenes.clear();
    this->viewports.clear();

}

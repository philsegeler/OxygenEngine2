#ifndef NRE_DATA_HANDLER_H
#define NRE_DATA_HANDLER_H

#include <Renderer/DataHandler/NRE_RenderData.h>

class NRE_DataHandler {
public:
    
    bool update(bool, bool);
    void clear();
    
    void handleMeshData(std::size_t, std::shared_ptr<OE_Mesh32>);
    void handleMaterialData(std::size_t, std::shared_ptr<OE_Material>);
    void handleCameraData(std::size_t, std::shared_ptr<OE_Camera>);
    void handleLightData(std::size_t, std::shared_ptr<OE_Light>);
    void handleVGroupData(std::size_t, std::size_t, std::shared_ptr<OE_Mesh32>);
    
    void handleSceneData(std::size_t, std::shared_ptr<OE_Scene>);
    void handleViewportData(std::size_t,std::shared_ptr<OE_ViewportConfig>);
    
    void deleteCamera(std::size_t);
    void deleteMaterial(std::size_t);
    void deleteMesh(std::size_t);
    
    
    bool load_spheres_or_bboxes{false};
    
    std::size_t loaded_viewport{0};
    std::map<std::size_t, NRE_CameraRenderData> cameras;
    std::map<std::size_t, NRE_MaterialRenderData> materials;
    std::map<std::size_t, NRE_VGroupRenderData> vgroups;
    std::map<std::size_t, NRE_MeshRenderData> meshes;
    
    std::map<std::size_t, NRE_DirectionalLightRenderData> dir_lights;
    std::map<std::size_t, NRE_PointLightRenderData> pt_lights;
    bool has_dir_lights_changed{false};
    bool has_pt_lights_changed{false};
    
    std::map<std::size_t, NRE_SceneRenderData> scenes;
    std::map<std::size_t, NRE_ViewportRenderData> viewports;    
    
    std::set<std::size_t> deleted_meshes;
    std::set<std::size_t> deleted_materials;
    std::set<std::size_t> deleted_cameras;
    std::set<std::size_t> deleted_scenes;
};


#endif

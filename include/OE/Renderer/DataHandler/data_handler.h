#ifndef NRE_DATA_HANDLER_H
#define NRE_DATA_HANDLER_H

#include <OE/Renderer/DataHandler/render_data.h>

namespace nre {
    class data_handler_t {
    public:
        bool update(bool, bool);
        void clear();

        void handle_mesh_data(std::size_t, std::shared_ptr<OE_Mesh32>);
        void handle_material_data(std::size_t, std::shared_ptr<OE_Material>);
        void handle_camera_data(std::size_t, std::shared_ptr<OE_Camera>);
        void handle_light_data(std::size_t, std::shared_ptr<OE_Light>);
        void handle_vgroup_data(std::size_t, std::size_t, std::shared_ptr<OE_Mesh32>);

        void handle_scene_data(std::size_t, std::shared_ptr<OE_Scene>);
        void handle_viewport_data(std::size_t, std::shared_ptr<OE_ViewportConfig>);

        void delete_camera(std::size_t);
        void delete_material(std::size_t);
        void delete_mesh(std::size_t);


        bool load_spheres_or_bboxes_{false};

        std::size_t                                        loaded_viewport_{0};
        std::map<std::size_t, nre::camera_render_data_t>   cameras_;
        std::map<std::size_t, nre::material_render_data_t> materials_;
        std::map<std::size_t, nre::vgroup_render_data_t>   vgroups_;
        std::map<std::size_t, nre::mesh_render_data_t>     meshes_;

        std::map<std::size_t, nre::directional_light_render_data_t> dir_lights_;
        std::map<std::size_t, nre::point_light_render_data_t>       pt_lights_;

        bool has_dir_lights_changed_{false};
        bool has_pt_lights_changed_{false};

        std::map<std::size_t, nre::scene_render_data_t>    scenes_;
        std::map<std::size_t, nre::viewport_render_data_t> viewports_;

        std::set<std::size_t> deleted_meshes_;
        std::set<std::size_t> deleted_materials_;
        std::set<std::size_t> deleted_cameras_;
        std::set<std::size_t> deleted_scenes_;
    };
}; // namespace nre

#endif

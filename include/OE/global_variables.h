#ifndef OE_GLOBAL_VARIABLES_H
#define OE_GLOBAL_VARIABLES_H

#include <OE/types/shared_index_map.h>
#include <OE/types/world.h>


namespace oe {

    extern oe::shared_index_map_t<OE_Scene>          scenes_list;
    extern oe::shared_index_map_t<OE_Object>         objects_list;
    extern oe::shared_index_map_t<OE_Material>       materials_list;
    extern oe::shared_index_map_t<OE_Texture>        textures_list;
    extern oe::shared_index_map_t<OE_TCM>            tcms_list;
    extern oe::shared_index_map_t<OE_ViewportConfig> viewports_list;


    // TODO: oe::world_manager_t
    // That class could also handle the shared_index_map_t's and garbage collection
    extern std::shared_ptr<OE_World> world;
    extern std::shared_ptr<OE_World> pending_world; // for loading a world
};                                                  // namespace oe

#endif

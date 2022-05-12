#include <OE/global_variables.h>

oe::shared_index_map_t<OE_Scene>          oe::scenes_list    = oe::shared_index_map_t<OE_Scene>();
oe::shared_index_map_t<OE_Object>         oe::objects_list   = oe::shared_index_map_t<OE_Object>();
oe::shared_index_map_t<OE_Material>       oe::materials_list = oe::shared_index_map_t<OE_Material>();
oe::shared_index_map_t<OE_Texture>        oe::textures_list  = oe::shared_index_map_t<OE_Texture>();
oe::shared_index_map_t<OE_TCM>            oe::tcms_list      = oe::shared_index_map_t<OE_TCM>();
oe::shared_index_map_t<OE_ViewportConfig> oe::viewports_list = oe::shared_index_map_t<OE_ViewportConfig>();


std::shared_ptr<OE_World> oe::world         = nullptr;
std::shared_ptr<OE_World> oe::pending_world = nullptr;

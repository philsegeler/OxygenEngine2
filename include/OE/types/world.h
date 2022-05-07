#ifndef OE_WORLD_H
#define OE_WORLD_H

#include <OE/types/scene.h>
#include <OE/types/shared_index_map.h>
#include <OE/types/viewport_config.h>

class OE_World : public OE_THREAD_SAFETY_OBJECT, public CSL_WriterBase {

    friend class CSL_Interpreter;
    friend class NRE_Renderer;

public:
    static oe::shared_index_map_t<OE_Scene>          scenesList;
    static oe::shared_index_map_t<OE_Object>         objectsList;
    static oe::shared_index_map_t<OE_Material>       materialsList;
    static oe::shared_index_map_t<OE_Texture>        texturesList;
    static oe::shared_index_map_t<OE_TCM>            tcmsList;
    static oe::shared_index_map_t<OE_ViewportConfig> viewportsList;



    OE_World();
    ~OE_World();

    void setup();

    std::string to_str() const;

    // protected:

    std::size_t loaded_scene;

    std::set<std::size_t> scenes;
    std::set<std::size_t> viewports;
    std::size_t           loaded_viewport;
};

#endif

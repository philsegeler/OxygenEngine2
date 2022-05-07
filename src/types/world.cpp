#include <OE/api_oe.h>
#include <OE/math_oe.h>
#include <OE/types/world.h>


using namespace std;

oe::shared_index_map_t<OE_Scene>          OE_World::scenesList    = oe::shared_index_map_t<OE_Scene>();
oe::shared_index_map_t<OE_Object>         OE_World::objectsList   = oe::shared_index_map_t<OE_Object>();
oe::shared_index_map_t<OE_Material>       OE_World::materialsList = oe::shared_index_map_t<OE_Material>();
oe::shared_index_map_t<OE_Texture>        OE_World::texturesList  = oe::shared_index_map_t<OE_Texture>();
oe::shared_index_map_t<OE_TCM>            OE_World::tcmsList      = oe::shared_index_map_t<OE_TCM>();
oe::shared_index_map_t<OE_ViewportConfig> OE_World::viewportsList = oe::shared_index_map_t<OE_ViewportConfig>();

OE_World::OE_World() {

    this->loaded_scene    = 0;
    this->loaded_viewport = 0;
}

OE_World::~OE_World() {

    for (auto& x : scenes)
        OE_World::scenesList.remove(x);
    for (auto& x : viewports)
        OE_World::viewportsList.remove(x);
}

void OE_World::setup() {

    lockMutex();


    // if no viewport config is defined, then use the default viewport config,
    // where the first scene and first camera are only defined
    if (this->viewports.size() == 0) {

        for (auto scene : OE_World::scenesList) {

            for (auto obj : scene->objects) {
                if (OE_World::objectsList[obj]->getType() == OE_OBJECT_CAMERA) {

                    // create and store default viewport config
                    std::shared_ptr<OE_ViewportConfig> vp_config = std::make_shared<OE_ViewportConfig>();

                    // add first found camera to default (0) layer
                    auto cam = OE_World::objectsList[obj];
                    vp_config->lockMutex();
                    vp_config->addCamera(cam.id(), 0);
                    vp_config->unlockMutex();

                    OE_World::viewportsList.force_append_now("default", vp_config);
                    this->viewports.insert(vp_config->id);
                    this->loaded_viewport = vp_config->id;
                    break;
                }
            }
            break;
        }
    }
    unlockMutex();
}


string OE_World::to_str() const {

    lockMutex();
    string output = outputTypeTag("World", {});
    output.append("\n");
    CSL_WriterBase::indent = CSL_WriterBase::indent + 1;

    for (const auto& x : this->scenes) {
        output.append(OE_World::scenesList[x]->to_str());
        output.append("\n");
    }

    for (const auto& x : this->viewports) {
        output.append(OE_World::viewportsList[x]->to_str());
        output.append("\n");
    }

    output.append(outputVar("loaded_scene", "\"" + OE_World::scenesList.get_name(this->loaded_scene) + "\""));
    output.append("\n");

    output.append(outputVar("loaded_viewport", "\"" + OE_World::viewportsList.get_name(this->loaded_viewport) + "\""));
    output.append("\n");

    CSL_WriterBase::indent = CSL_WriterBase::indent - 1;
    output.append(outputClosingTag("World"));
    unlockMutex();
    return output;
}

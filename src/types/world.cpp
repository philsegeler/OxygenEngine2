#include <OE/global_variables.h>


using namespace std;

OE_World::OE_World() {

    this->loaded_scene    = 0;
    this->loaded_viewport = 0;
}

OE_World::~OE_World() {

    for (auto& x : scenes)
        oe::scenes_list.remove(x);
    for (auto& x : viewports)
        oe::viewports_list.remove(x);
}

std::size_t OE_World::get_loaded_viewport() {
    lockMutex();
    std::size_t output = this->loaded_viewport;
    unlockMutex();
    return output;
}

void OE_World::setup() {

    lockMutex();


    // if no viewport config is defined, then use the default viewport config,
    // where the first scene and first camera are only defined
    if (this->viewports.size() == 0) {

        for (auto scene : oe::scenes_list) {

            for (auto obj : scene->objects) {
                if (oe::objects_list[obj]->getType() == OE_OBJECT_CAMERA) {

                    // create and store default viewport config
                    std::shared_ptr<OE_ViewportConfig> vp_config = std::make_shared<OE_ViewportConfig>();

                    // add first found camera to default (0) layer
                    auto cam = oe::objects_list[obj];
                    vp_config->lockMutex();
                    vp_config->addCamera(cam.get_id(), 0);
                    vp_config->unlockMutex();

                    oe::viewports_list.force_append_now("default", vp_config);
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
        output.append(oe::scenes_list[x]->to_str());
        output.append("\n");
    }

    for (const auto& x : this->viewports) {
        output.append(oe::viewports_list[x]->to_str());
        output.append("\n");
    }

    output.append(outputVar("loaded_scene", "\"" + oe::scenes_list.get_name(this->loaded_scene) + "\""));
    output.append("\n");

    output.append(outputVar("loaded_viewport", "\"" + oe::viewports_list.get_name(this->loaded_viewport) + "\""));
    output.append("\n");

    CSL_WriterBase::indent = CSL_WriterBase::indent - 1;
    output.append(outputClosingTag("World"));
    unlockMutex();
    return output;
}

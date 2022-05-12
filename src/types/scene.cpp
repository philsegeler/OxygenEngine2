#include <OE/global_variables.h>
#include <OE/types/scene.h>

using namespace std;

std::atomic<std::size_t> OE_Scene::current_id(0);

OE_Scene::OE_Scene() : id(++OE_Scene::current_id) {
}


OE_Scene::~OE_Scene() {
    /*for (auto object : this->objects){
        oe::objects_list.remove(object);
    }
    this->objects.clear();

    for (auto material : this->materials){
        oe::materials_list.remove(material);
    }
    this->materials.clear();

    for (auto tcm : this->texture_CMs){
        oe::tcms_list.remove(tcm);
    }
    this->texture_CMs.clear();

    for (auto texture : this->textures){
        oe::textures_list.remove(texture);
    }
    this->textures.clear();*/
}

string OE_Scene::to_str() const {

    lockMutex();
    string output = outputTypeTag("Scene", {{"name", "\"" + oe::scenes_list.get_name(this->id) + "\""}});
    output.append("\n");
    CSL_WriterBase::indent = CSL_WriterBase::indent + 1;

    for (const auto& x : this->textures) {
        output.append(oe::textures_list[x]->to_str());
        output.append("\n");
    }

    for (const auto& x : this->texture_CMs) {
        output.append(oe::tcms_list[x]->to_str());
        output.append("\n");
    }

    for (const auto& x : this->materials) {
        output.append(oe::materials_list[x]->to_str());
        output.append("\n");
    }

    for (const auto& x : this->objects) {
        if (oe::objects_list[x]->getType() == OE_OBJECT_MESH) {
            output.append(oe::objects_list[x]->to_str());
            output.append("\n");
        }
    }
    for (const auto& x : this->objects) {
        if (oe::objects_list[x]->getType() == OE_OBJECT_LIGHT) {
            output.append(oe::objects_list[x]->to_str());
            output.append("\n");
        }
    }
    for (const auto& x : this->objects) {
        if (oe::objects_list[x]->getType() == OE_OBJECT_CAMERA) {
            output.append(oe::objects_list[x]->to_str());
            output.append("\n");
        }
    }

    CSL_WriterBase::indent = CSL_WriterBase::indent - 1;
    output.append(outputClosingTag("Scene"));
    unlockMutex();
    return output;
}

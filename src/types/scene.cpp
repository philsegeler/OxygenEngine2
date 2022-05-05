#include <OE/types/scene.h>
#include <OE/types/world.h>

using namespace std;

std::atomic<std::size_t> OE_Scene::current_id(0);

OE_Scene::OE_Scene() : id(++OE_Scene::current_id) {
}


OE_Scene::~OE_Scene() {
    /*for (auto object : this->objects){
        OE_World::objectsList.remove(object);
    }
    this->objects.clear();

    for (auto material : this->materials){
        OE_World::materialsList.remove(material);
    }
    this->materials.clear();

    for (auto tcm : this->texture_CMs){
        OE_World::tcmsList.remove(tcm);
    }
    this->texture_CMs.clear();

    for (auto texture : this->textures){
        OE_World::texturesList.remove(texture);
    }
    this->textures.clear();*/
}

string OE_Scene::to_str() const {

    lockMutex();
    string output = outputTypeTag("Scene", {{"name", "\"" + OE_World::scenesList.get_name(this->id) + "\""}});
    output.append("\n");
    CSL_WriterBase::indent = CSL_WriterBase::indent + 1;

    for (const auto& x : this->textures) {
        output.append(OE_World::texturesList[x]->to_str());
        output.append("\n");
    }

    for (const auto& x : this->texture_CMs) {
        output.append(OE_World::tcmsList[x]->to_str());
        output.append("\n");
    }

    for (const auto& x : this->materials) {
        output.append(OE_World::materialsList[x]->to_str());
        output.append("\n");
    }

    for (const auto& x : this->objects) {
        if (OE_World::objectsList[x]->getType() == OE_OBJECT_MESH) {
            output.append(OE_World::objectsList[x]->to_str());
            output.append("\n");
        }
    }
    for (const auto& x : this->objects) {
        if (OE_World::objectsList[x]->getType() == OE_OBJECT_LIGHT) {
            output.append(OE_World::objectsList[x]->to_str());
            output.append("\n");
        }
    }
    for (const auto& x : this->objects) {
        if (OE_World::objectsList[x]->getType() == OE_OBJECT_CAMERA) {
            output.append(OE_World::objectsList[x]->to_str());
            output.append("\n");
        }
    }

    CSL_WriterBase::indent = CSL_WriterBase::indent - 1;
    output.append(outputClosingTag("Scene"));
    unlockMutex();
    return output;
}

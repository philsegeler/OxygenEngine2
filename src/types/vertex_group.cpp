#include <OE/global_variables.h>
#include <OE/types/material.h>
#include <OE/types/object.h>
#include <OE/types/vertex_group.h>

using namespace std;

std::atomic<std::size_t>       OE_VertexGroup::current_id(0);
unordered_map<size_t, string>  OE_VertexGroup::id2name;
OE_Name2ID_ContainerDeprecated OE_VertexGroup::name2id = OE_Name2ID_ContainerDeprecated(&OE_VertexGroup::id2name);

OE_VertexGroup::OE_VertexGroup() : id(++OE_VertexGroup::current_id) {

    this->material_id = 0;
    this->bone_id     = 0;
    this->visible     = true;

    OE_VertexGroup::id2name[this->id] = "noname_" + to_string(this->id);
}

OE_VertexGroup::OE_VertexGroup(const std::string& name) : id(++OE_VertexGroup::current_id) {

    this->material_id = 0;
    this->bone_id     = 0;
    this->visible     = true;

    OE_VertexGroup::id2name[this->id] = name;
}

OE_VertexGroup::~OE_VertexGroup() {
    OE_VertexGroup::id2name.erase(this->id);
}

string OE_VertexGroup::to_str() const {

    lockMutex();
    string temp1 =
        outputTypeTag("VertexGroup", {{"name", "\"" + id2name[this->id] + "\""}, {"visible", convert((int)visible)}});
    CSL_WriterBase::indent = CSL_WriterBase::indent + 1;

    string temp3 = outputList("polygons", this->polygons);

    string temp4;
    if (this->material_id != 0)
        temp4 = outputVar("material_id", "\"" + oe::materials_list.get_name(this->material_id) + "\"");
    else {
        temp4 = outputVar("material_id", "\"\"");
    }
    string temp5;
    if (this->bone_id != 0)
        temp5 = outputVar("bone_id", "\"\"");
    else
        temp5 = outputVar("bone_id", "\"\"");

    CSL_WriterBase::indent = CSL_WriterBase::indent - 1;
    string temp6           = outputClosingTag("VertexGroup");
    unlockMutex();
    return CSL_Join("\n", {temp1, temp4, temp5, temp3, temp6});
}

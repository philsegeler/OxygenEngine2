#include <OE/global_variables.h>
#include <OE/types/camera.h>
#include <OE/types/texture.h>

using namespace std;


std::atomic<std::size_t> OE_Texture::current_id(0);

OE_Texture::OE_Texture() : id(++OE_Texture::current_id) {

    this->source      = 0;
    this->path        = "";
    this->camera      = 0;
    this->custom_data = nullptr;
}

OE_Texture::~OE_Texture() {
}

string OE_Texture::to_str() const {
    lockMutex();
    string output = outputTypeTag("Texture", {{"name", "\"" + oe::textures_list.get_name(this->id) + "\""}});
    output.append("\n");
    CSL_WriterBase::indent = CSL_WriterBase::indent + 1;

    output.append(outputVar("source", convert(this->source)));
    output.append("\n");

    output.append(outputVar("path", "\"" + this->path + "\""));
    output.append("\n");

    if (this->camera != 0) {
        output.append(outputVar("camera", "\"\""));
        output.append("\n");
    }
    else {
        output.append(outputVar("camera", "\"" + oe::objects_list.get_name(this->camera) + "\""));
        output.append("\n");
    }

    CSL_WriterBase::indent = CSL_WriterBase::indent - 1;
    output.append(outputClosingTag("Texture"));
    unlockMutex();
    return output;
}

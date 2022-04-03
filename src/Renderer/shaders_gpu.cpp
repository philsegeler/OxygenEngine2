#include <OE/Renderer/GL3/shaders_gl3.h>
#include <OE/Renderer/GLES2/shaders_gles2.h>
#include <OE/Renderer/api_gpu.h>
#include <bitset>
#include <iostream>
#include <sstream>
using namespace std;
using namespace nre::gpu;


// General boilerplate

std::string nre::gpu::shader_base::shader_prefix = "";

nre::gpu::shader_base::~shader_base() {
}

void nre::gpu::shader_base::init(SHADER_BACKEND backend, int major, int minor) {


    if (backend == GLES) {
        nre::gpu::shader_base::shader_prefix = "#version 300 es \n";
    }
    else if (backend == GL) {

        if (major == 3 && minor == 1) {
            nre::gpu::shader_base::shader_prefix = "#version 140 \n"
                                                   "#extension GL_ARB_explicit_attrib_location"
                                                   ": require\n";
        }
        else if (major == 3 && minor >= 2) {
            nre::gpu::shader_base::shader_prefix = "#version 150 core \n"
                                                   "#extension GL_ARB_explicit_attrib_location"
                                                   ": require\n";
        }
        else {
            cout << "WTF" << endl;
        }
    }
    else if (backend == GLES2) {
        nre::gpu::shader_base::shader_prefix = "#version 100 \n";
    }
    else {
        cout << "Undefined NRE GPU Shader backend. Nothing is rendered" << endl;
    }
}


std::string nre::gpu::shader_base::gen_shader() const {
    return "";
}

std::string nre::gpu::shader_base::info() const {
    return "";
}

// Vertex Shader

nre::gpu::vertex_shader::vertex_shader() {
}

nre::gpu::vertex_shader::~vertex_shader() {
}

bool nre::gpu::vertex_shader::operator==(const nre::gpu::vertex_shader& other) const {
    return std::tie(this->fullscreenQuad, this->type, this->num_of_uvs) ==
           std::tie(other.fullscreenQuad, other.type, other.num_of_uvs);
}


std::string nre::gpu::vertex_shader::gen_shader() const {
    if (nre::gpu::get_api() == GL || nre::gpu::get_api() == GLES)
        return NRE_GenGL3VertexShader(*this);
    else if (nre::gpu::get_api() == GLES2)
        return NRE_GenGLES2VertexShader(*this);
    else
        return "";
}

std::string nre::gpu::vertex_shader::info() const {

    stringstream ss;
    switch (this->type) {
    case VS_UNDEFINED:
        ss << "VS_UNDEFINED";
        break;
    case VS_Z_PREPASS:
        ss << "VS_Z_PREPASS";
        break;
    case VS_REGULAR:
        ss << "VS_REGULAR";
        break;
    case VS_BOUNDING_BOX:
        ss << "VS_BOUNDING_BOX";
        break;
    case VS_BOUNDING_SPHERE:
        ss << "VS_BOUNDING_SPHERE";
        break;
    case VS_LIGHT:
        ss << "VS_LIGHT";
        break;
    }
    ss << " " << this->fullscreenQuad << " " << this->num_of_uvs;
    return ss.str();
}


size_t nre::gpu::vertex_shader::gen_hash() const {
    // should only use the first 32
    std::bitset<64> output_bits(this->fullscreenQuad);

    output_bits |= (num_of_uvs & 7) << 1;
    output_bits |= (this->type << 4);


    return output_bits.to_ullong();
}

// Fragment/Pixel Shader

nre::gpu::pixel_shader::pixel_shader() {
}

nre::gpu::pixel_shader::~pixel_shader() {
}

bool nre::gpu::pixel_shader::operator==(const nre::gpu::pixel_shader& other) const {
    return std::tie(this->type, this->num_of_uvs) == std::tie(other.type, other.num_of_uvs);
}

std::string nre::gpu::pixel_shader::gen_shader() const {
    if (nre::gpu::get_api() == GL || nre::gpu::get_api() == GLES)
        return NRE_GenGL3PixelShader(*this);
    else if (nre::gpu::get_api() == GLES2)
        return NRE_GenGLES2PixelShader(*this);
    else
        return "";
    return "";
}

std::string nre::gpu::pixel_shader::info() const {
    stringstream ss;
    switch (this->type) {
    case FS_UNDEFINED:
        ss << "FS_UNDEFINED";
        break;
    case FS_SIMPLE:
        ss << "FS_SIMPLE";
        break;
    case FS_GAMMA:
        ss << "FS_GAMMA";
        break;
    case FS_MATERIAL:
        ss << "FS_MATERIAL";
        break;
    case FS_NORMALS:
        ss << "FS_NORMALS";
        break;
    case FS_SIMPLE_LIGHT:
        ss << "FS_SIMPLE_LIGHT";
        break;
    case FS_DIR_LIGHT:
        ss << "FS_DIR_LIGHT";
        break;
    case FS_INDIR_LIGHTS:
        ss << "FS_INDIR_LIGHTS";
        break;
    case FS_LIGHT_INDEX:
        ss << "FS_LIGHT_INDEX";
        break;
    }
    ss << " " << this->num_of_uvs;
    return ss.str();
}


size_t nre::gpu::pixel_shader::gen_hash() const {
    // should only use the last 32
    std::bitset<64> output_bits(0);

    output_bits |= (num_of_uvs & 7) << 29;
    output_bits |= (this->type << 26);
    output_bits = output_bits << 32;

    return output_bits.to_ullong();
}

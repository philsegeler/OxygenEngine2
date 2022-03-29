#include <Renderer/GL3/shaders_gl3.h>
#include <Renderer/GLES2/shaders_gles2.h>
#include <Renderer/api_gpu.h>
#include <iostream>
#include <sstream>

using namespace std;
using namespace nre::gpu;


// General boilerplate

std::string nre::gpu::shader_base::shader_prefix = "";

nre::gpu::shader_base::~shader_base(){
    
}

void nre::gpu::shader_base::init(SHADER_BACKEND backend, int major, int minor){
    
    
    if (backend == GLES){
        nre::gpu::shader_base::shader_prefix = "#version 300 es \n";
    }
    else if (backend == GL){
        
        if (major == 3 && minor == 1){
            nre::gpu::shader_base::shader_prefix = "#version 140 \n"
                                 "#extension GL_ARB_explicit_attrib_location"
                                 ": require\n";
        } 
        else if (major == 3 && minor >= 2){
            nre::gpu::shader_base::shader_prefix = "#version 150 core \n"
                                 "#extension GL_ARB_explicit_attrib_location"
                                 ": require\n";
        }
        else {
            cout << "WTF" << endl;
        }
    }
    else if (backend == GLES2){
        nre::gpu::shader_base::shader_prefix = "#version 100 \n";
    }
    else {
        cout << "Undefined NRE GPU Shader backend. Nothing is rendered" << endl;
    }
    
}


std::string nre::gpu::shader_base::gen_shader(){
    return "";
}

std::string nre::gpu::shader_base::info(){
    return "";
}

// Vertex Shader

nre::gpu::vertex_shader::vertex_shader(){
    
}

nre::gpu::vertex_shader::~vertex_shader(){
    
}

bool nre::gpu::vertex_shader::operator< (const nre::gpu::vertex_shader& other) const{
    if (this->num_of_uvs < other.num_of_uvs){
        return true;
    }
    else if (this->num_of_uvs > other.num_of_uvs){
        return false;
    }
    else {
        //both are equal
        if (this->type < other.type){
            return true;
        } 
        else if (this->type > other.type){
            return false;
        }
        else {
            return this->fullscreenQuad < other.fullscreenQuad;
        }
    }
    return false;
}

bool nre::gpu::vertex_shader::operator == (const nre::gpu::vertex_shader& other) const{
    return (this->fullscreenQuad == other.fullscreenQuad) && (this->type == other.type) && (this->num_of_uvs == other.num_of_uvs);
}

std::string nre::gpu::vertex_shader::gen_shader(){
    if (nre::gpu::get_api() == GL || nre::gpu::get_api() == GLES)
        return NRE_GenGL3VertexShader(*this);
    else if (nre::gpu::get_api() == GLES2)
        return NRE_GenGLES2VertexShader(*this);
    else
        return "";
}

std::string nre::gpu::vertex_shader::info(){
    
    stringstream ss;
    switch(this->type){
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
    
    return ss.str();
}

// Fragment/Pixel Shader

nre::gpu::pixel_shader::pixel_shader(){
    
}

nre::gpu::pixel_shader::~pixel_shader(){
    
}

bool nre::gpu::pixel_shader::operator< (const nre::gpu::pixel_shader& other) const{
    if (this->num_of_uvs < other.num_of_uvs){
        return true;
    }
    else if (this->num_of_uvs > other.num_of_uvs){
        return false;
    }
    else {
        //both are equal
        return this->type < other.type;
    }
    return false;
}

std::string nre::gpu::pixel_shader::gen_shader(){
    if (nre::gpu::get_api() == GL || nre::gpu::get_api() == GLES)
        return NRE_GenGL3PixelShader(*this);
    else if (nre::gpu::get_api() == GLES2)
        return NRE_GenGLES2PixelShader(*this);
    else
        return "";
    return "";
}

std::string nre::gpu::pixel_shader::info(){
    stringstream ss;
    switch(this->type){
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
    
    return ss.str();
}

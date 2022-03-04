#include <OE_Math.h>
#include <Renderer/NRE_RendererMain.h>

using namespace std;


std::vector<float> NRE_MeshRenderData::genBoundingBoxVBO(){
    return OE_GetBoundingBoxVertexBuffer(max_x, min_x, max_y, min_y, max_z, min_z);
}

OE_Vec4 NRE_CameraRenderData::get_position(){
    return OE_Vec4(this->model_mat[3][0], this->model_mat[3][1], this->model_mat[3][2], this->model_mat[3][3]);
}

OE_Vec4 NRE_MaterialRenderData::get_mat_diffuse(){
    return OE_Vec4(this->data[0], this->data[1], this->data[2], this->data[3]);
}

float NRE_MaterialRenderData::get_mat_specular_hardness(){
    return this->data[9];
}

std::vector<float> NRE_MeshRenderData::get_scaling_min_data(){
    std::vector<float> output; output.reserve(4);
    output.push_back(this->min_x);
    output.push_back(this->min_y);
    output.push_back(this->min_z);
    output.push_back(this->min_radius);
    return output;
}

std::vector<float> NRE_MeshRenderData::get_scaling_max_data(){
    std::vector<float> output; output.reserve(4);
    output.push_back(this->max_x);
    output.push_back(this->max_y);
    output.push_back(this->max_z);
    output.push_back(this->max_radius);
    return output;
}

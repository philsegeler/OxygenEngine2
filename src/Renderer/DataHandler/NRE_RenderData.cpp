#include <OE_Math.h>
#include <Renderer/NRE_RendererMain.h>

using namespace std;


std::vector<float> NRE_MeshRenderData::genBoundingBoxVBO(){
    return OE_GetBoundingBoxVertexBuffer(max_x, min_x, max_y, min_y, max_z, min_z);
}


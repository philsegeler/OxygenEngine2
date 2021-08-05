#include <Renderer/NRE_RendererUtils.h>

bool NRE_RenderGroup::operator < (const NRE_RenderGroup& other) const{
    
    // sort by camera
    if (this->camera < other.camera){
        return true;
    }
    else if (this->camera > other.camera){
        return false;
    }
    else{
        // sort by material (program) since pipeline/texture changes
        // are expensive
        if (this->material < other.material){
            return true;
        }
        else if (this->material > other.material){
            return false;
        }
        else{
            // sort by mesh, to minimize changing VAO
            // and Mesh Buffers
            if (this->mesh < other.mesh){
                return true;
            }
            else if (this->mesh > other.mesh){
                return false;
            }
            else{
                // finally sort by vgroup since changing Index Buffers should be cheap
                if (this->vgroup < other.vgroup){
                    return true;
                }
                else if (this->vgroup >= other.vgroup){
                    return false;
                }
            }
        }
    }
    return false;
}

NRE_PointLightDrawCall::NRE_PointLightDrawCall(std::size_t id_in, float z_in, std::size_t priority_in){
    this->id = id_in;
    this->z = z_in;
    this->priority = priority_in;
}

bool NRE_PointLightDrawCall::operator > (const NRE_PointLightDrawCall& other) const{
    if (this->priority == other.priority){
        if (std::abs(this->z - other.z) < 0.0000001f){
            return this->id > other.id;
        }
        else{
            return this->z > other.z;
        }
    }
    else{
        return this->priority > other.priority;
    }
    return false;
}
 

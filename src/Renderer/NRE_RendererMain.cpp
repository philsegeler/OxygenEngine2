#include <Renderer/NRE_RendererMain.h>

using namespace std;

NRE_Renderer::NRE_Renderer(){
    
}

NRE_Renderer::~NRE_Renderer(){
   
}
    
bool NRE_Renderer::init(){
    if (api != nullptr)
        delete api;
    this->api = new NRE_GL3_API();
    if (!this->screen->isES){
        if (this->screen->major == 3 && this->screen->minor == 1){
            this->gl_shader_prefix = "#version 140 \n"
                                 "#extension GL_ARB_explicit_attrib_location"
                                 ": require\n";
        } else{
            this->gl_shader_prefix = "#version 150 \n"
                                 "#extension GL_ARB_explicit_attrib_location"
                                 ": require\n";
        }
        
    } else{
        this->gl_shader_prefix = "#version 300 es \nprecision highp float; \n";
    }
    
    return true;
}

bool NRE_Renderer::updateSingleThread(){
    for (auto &x: this->render_groups){
        this->drawRenderGroup(&x);
    }
    return true;
}

void NRE_Renderer::drawRenderGroup(NRE_RenderGroup *ren_group){
    
    if (!ren_group->isSetup){
       //cout << "Setting up Render group" << ren_group->camera << " " << ren_group->vgroup << " " << ren_group->mesh << endl;
        ren_group->program = this->api->newProgram();
        ren_group->isSetup = true;
        this->api->setProgramVS(ren_group->program, string(gl_shader_prefix + string(NRE_Shader(
            
            layout (location=0) in vec3 oe_position;
            layout (location=1) in vec3 oe_normals;
            
            out vec3 position;
            out vec3 normals;
            
            layout(std140) uniform OE_Camera{
                mat4 PV_Matrix;
            };
            
            layout(std140) uniform OE_Mesh32{
                mat4 Model_Matrix;
            };
            
            void main(){
                normals = mat3(Model_Matrix)*oe_normals;
                mat4 final_mat = PV_Matrix*Model_Matrix;
                
                vec4 temp_position = Model_Matrix*vec4(oe_position, 1.0);
                position = temp_position.xyz;
                gl_Position = final_mat*vec4(oe_position, 1.0);
            }
            
        ))));
        this->api->setProgramFS(ren_group->program,  string(gl_shader_prefix +  string(NRE_Shader(
            
            in vec3 position;
            in vec3 normals;
            
            out vec4 fragColor;
            
            void main(){
                fragColor = vec4(abs(normals), 1.0);
            }
            
        ))));
        
        this->api->setupProgram(ren_group->program);
        this->api->setProgramUniformSlot(ren_group->program, "OE_Camera", 0);
        this->api->setProgramUniformSlot(ren_group->program, "OE_Mesh32", 1);
        
    }
    this->api->setUniformState(this->meshes[ren_group->mesh].ubo, 1, 0, 0);
    this->api->setUniformState(this->cameras[ren_group->camera].ubo, 0, 0, 0);
    this->api->draw(ren_group->program, this->meshes[ren_group->mesh].vao, this->vgroups[ren_group->vgroup].ibo);
}


bool NRE_Renderer::updateMultiThread(OE_Task*, int){
    return false;
}

void NRE_Renderer::destroy(){
    if (api != nullptr)
        delete api;
}    

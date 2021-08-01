#include <Renderer/NRE_GPU_API.h>
#include <Renderer/GL3/NRE_GL3_API.h>

uint32_t nre::gpu::x = 0;
uint32_t nre::gpu::y = 0;
void* nre::gpu::api = nullptr;
std::atomic<bool>  nre::gpu::use_wireframe = false;
nre::gpu::info_struct nre::gpu::backend_info = nre::gpu::info_struct();

nre::gpu::vertex_layout_input::vertex_layout_input(){
    
}

nre::gpu::vertex_layout_input::vertex_layout_input(std::size_t vbo, std::size_t offseta, unsigned int amounta, unsigned int stridea){
    this->vertex_buffer = vbo;
    this->offset = offseta;
    this->amount = amounta;
    this->stride = stridea;
}

nre::gpu::info_struct nre::gpu::get_backend_info(){
    return nre::gpu::backend_info;
}

std::string nre::gpu::get_underlying_api_name(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->getRenderingAPI();
            
        default:
            return "undefined";
    }
}

nre::gpu::SHADER_BACKEND nre::gpu::get_api(){
    return nre::gpu::backend_info.underlying_api;
}


std::size_t nre::gpu::new_vertex_buf(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->newVertexBuffer();
            
        default:
            return 0;
    }
}

std::size_t nre::gpu::new_program(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->newProgram();
            
        default:
            return 0;
    }
}

std::size_t nre::gpu::new_vertex_layout(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->newVertexLayout();
            
        default:
            return 0;
    }
}

std::size_t nre::gpu::new_index_buf(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->newIndexBuffer();
            
        default:
            return 0;
    }
}

std::size_t nre::gpu::new_uniform_buf(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->newUniformBuffer();
            
        default:
            return 0;
    }
}

std::size_t nre::gpu::new_framebuffer(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->newFrameBuffer();
            
        default:
            return 0;
    }
}

std::size_t nre::gpu::new_texture(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->newTexture();
            
        default:
            return 0;
    }
}

std::size_t nre::gpu::new_renderbuffer(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->newRenderBuffer();
            
        default:
            return 0;
    }
}

// init update destroy
bool nre::gpu::init(SHADER_BACKEND backend_in, int major, int minor){
    
    nre::gpu::info_struct info_backend;
    info_backend.underlying_api = backend_in;
    info_backend.major = major;
    info_backend.minor = minor;
    nre::gpu::backend_info = info_backend;
    
    nre::gpu::shader_base::init(backend_in, major, minor);
    nre::gpu::backend_info.has_init = true;
    
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             nre::gpu::api = static_cast<void*>(new NRE_GL3_API());
             break;
        default: return false;
    }
    
    
    return true;
}

void nre::gpu::update(uint32_t x, uint32_t y){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->update(x, y);
             break;
        default: return;
    }
}

void nre::gpu::destroy(){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->destroy();
             delete static_cast<NRE_GL3_API*>(nre::gpu::api);
             nre::gpu::api = nullptr;
             break;
        default: return;
    }
}

// renderbuffers
void nre::gpu::set_renderbuffer_mode(std::size_t id, nre::gpu::TEXTURE_TYPE a_type, int x, int y){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setRenderBufferType(id, a_type, x, y);
             break;
        default: return;
    }
}

void nre::gpu::set_framebuffer_renderbuffer(std::size_t fbo_id, std::size_t rbo_id, int slot){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setFrameBufferRenderBuffer(fbo_id, rbo_id, slot);
             break;
        default: return;
    }
}

// vertex buffers
void nre::gpu::set_vertex_buf_memory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setVertexBufferMemory(id, memory_size, buf_usage);
             break;
        default: return;
    }
}

void nre::gpu::set_vertex_buf_data(std::size_t id, const std::vector<float>& data, std::size_t offset){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setVertexBufferData(id, data, offset);
             break;
        default: return;
    }
}

void nre::gpu::set_vertex_buf_memory_and_data(std::size_t id, const std::vector<float>& data, BUFFER_USAGE buf_usage){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setVertexBufferMemoryData(id, data, buf_usage);
             break;
        default: return;
    }
}

void nre::gpu::del_vertex_buf(std::size_t id){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->deleteVertexBuffer(id);
             break;
        default: return;
    }
}

// index buffers
void nre::gpu::set_index_buf_memory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setIndexBufferMemory(id, memory_size, buf_usage);
             break;
        default: return;
    }
}

void nre::gpu::set_index_buf_data(std::size_t id, const std::vector<uint32_t>& data, std::size_t offset){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setIndexBufferData(id, data, offset);
             break;
        default: return;
    }
}

void nre::gpu::set_index_buf_memory_and_data(std::size_t id, const std::vector<uint32_t>& data, BUFFER_USAGE buf_usage){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setIndexBufferMemoryData(id, data, buf_usage);
             break;
        default: return;
    }
}

void nre::gpu::del_index_buf(std::size_t id){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->deleteIndexBuffer(id);
             break;
        default: return;
    }
}

// uniform buffers
void nre::gpu::set_uniform_buf_memory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setUniformBufferMemory(id, memory_size, buf_usage);
             break;
        default: return;
    }
}

void nre::gpu::set_uniform_buf_data(std::size_t id, const std::vector<float>& data, std::size_t offset){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setUniformBufferData(id, data, offset);
             break;
        default: return;
    }
}

void nre::gpu::set_uniform_buf_data(std::size_t id, const std::vector<uint32_t>& data, std::size_t offset){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setUniformBufferData(id, data, offset);
             break;
        default: return;
    }
}

void nre::gpu::set_uniform_buf_memory_and_data(std::size_t id, const std::vector<uint32_t>& data, BUFFER_USAGE buf_usage){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setUniformBufferMemory(id, data.size(), buf_usage);
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setUniformBufferData(id, data, 0);
             break;
        default: return;
    }
}

void nre::gpu::set_uniform_buf_memory_and_data(std::size_t id, const std::vector<float>& data, BUFFER_USAGE buf_usage){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setUniformBufferMemory(id, data.size(), buf_usage);
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setUniformBufferData(id, data, 0);
             break;
        default: return;
    }
}

void nre::gpu::del_uniform_buf(std::size_t id){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->deleteUniformBuffer(id);
             break;
        default: return;
    }
}

void nre::gpu::set_uniform_buf_state(std::size_t id, std::size_t program, int slot, std::size_t offset, std::size_t length){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setUniformBlockState(id, program, slot, offset, length);
             break;
        default: return;
    }
}
// program uniform buffer/texture state
void nre::gpu::set_program_uniform_buf_slot(std::size_t id, std::string ubo, int slot){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setProgramUniformBlockSlot(id, ubo, slot);
             break;
        default: return;
    }
}
int  nre::gpu::get_program_uniform_buf_slot(std::size_t id, std::string name){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->getProgramUniformBlockSlot(id, name);
            
        default:
            return -2;
    }
}
void nre::gpu::set_program_texture_slot(std::size_t id, std::string name, int slot){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setProgramTextureSlot(id, name, slot);
             break;
        default: return;
    }
}

// program uniform data
void nre::gpu::set_program_uniform_data(std::size_t id, std::string name, uint32_t data){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setProgramUniformData(id, name, data);
             break;
        default: return;
    }
}
void nre::gpu::set_program_uniform_data(std::size_t id, std::string name, float data){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setProgramUniformData(id, name, data);
             break;
        default: return;
    }
}
void nre::gpu::set_program_uniform_data(std::size_t id, std::string name, const std::vector<uint32_t>& data){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setProgramUniformData(id, name, data);
             break;
        default: return;
    }
}
void nre::gpu::set_program_uniform_data(std::size_t id, std::string name, const std::vector<float>& data){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             //static_cast<NRE_GL3_API*>(nre::gpu::api)->setProgramUniformData(id, name, data);
             break;
        default: return;
    }
}
void nre::gpu::set_program_uniform_slot(std::size_t id, std::string name, int slot){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             //static_cast<NRE_GL3_API*>(nre::gpu::api)->setProgramUniformSlot(id, name, slot);
             break;
        default: return;
    }
}
int  nre::gpu::get_program_uniform_slot(std::size_t id, std::string name){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            return static_cast<NRE_GL3_API*>(nre::gpu::api)->getProgramUniformSlot(id, name);
            
        default:
            return -2;
    }
}

// vertex layout
void nre::gpu::set_vertex_layout_format(std::size_t id, std::vector<vertex_layout_input> vao_input){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setVertexLayoutFormat(id, vao_input);
             break;
        default: return;
    }
}
void nre::gpu::del_vertex_layout(std::size_t id){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->deleteVertexLayout(id);
             break;
        default: return;
    }
}

// texture format
void nre::gpu::set_texture_format(std::size_t id, TEXTURE_TYPE textype, TEXTURE_FILTER texfil, uint32_t x, uint32_t y, int mipmaps){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setTextureFormat(id, textype, texfil, x, y, mipmaps);
             break;
        default: return;
    }
}
void nre::gpu::set_texture_slot(std::size_t id, int slot){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setTextureSlot(id, slot);
             break;
        default: return;
    }
}
void nre::gpu::del_texture(std::size_t id){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->deleteTexture(id);
             break;
        default: return;
    }
}

// framebuffer texture
void nre::gpu::set_framebuffer_texture(std::size_t id, std::size_t tex, int slot){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setFrameBufferTexture(id, tex, slot);
             break;
        default: return;
    }
}
void nre::gpu::copy_framebuffer(std::size_t src, std::size_t target, FRAMEBUFFER_COPY method){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->copyFrameBuffer(src, target, method);
             break;
        default: return;
    }
}
void nre::gpu::use_framebuffer(std::size_t id){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->useFrameBuffer(id);
             break;
        default: return;
    }
}
void nre::gpu::clear_framebuffer(std::size_t id, nre::gpu::FRAMEBUFFER_COPY fbcopy, float alpha){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->clearFrameBuffer(id, fbcopy, alpha);
             break;
        default: return;
    }
}
void nre::gpu::del_framebuffer(std::size_t id){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->deleteFrameBuffer(id);
             break;
        default: return;
    }
}

// vertex shaders
void nre::gpu::set_program_vertex_shader(std::size_t id, nre::gpu::vertex_shader vs){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setProgramVS(id, vs);
             break;
        default: return;
    }
}
void nre::gpu::set_program_pixel_shader(std::size_t id, nre::gpu::pixel_shader fs){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setProgramFS(id, fs);
             break;
        default: return;
    }
}
void nre::gpu::setup_program(std::size_t id){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setupProgram(id);
             break;
        default: return;
    }
}
void nre::gpu::del_program(std::size_t id){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->deleteProgram(id);
             break;
        default: return;
    }
}

void nre::gpu::draw(nre::gpu::draw_call dcall){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            if ((dcall.offset == 0) and (dcall.amount == 0)){
                if (dcall.index_buf == 0)
                    static_cast<NRE_GL3_API*>(nre::gpu::api)->draw(dcall.program, dcall.vertex_layout);
                else
                    static_cast<NRE_GL3_API*>(nre::gpu::api)->draw(dcall.program, dcall.vertex_layout, dcall.index_buf);
            }
            else{
                if (dcall.index_buf == 0)
                    static_cast<NRE_GL3_API*>(nre::gpu::api)->draw(dcall.program, dcall.vertex_layout, dcall.offset, dcall.amount);
                else
                    static_cast<NRE_GL3_API*>(nre::gpu::api)->draw(dcall.program, dcall.vertex_layout, dcall.index_buf, dcall.offset, dcall.amount);
            }
            break;
        default: return;
    }
}

void nre::gpu::draw(std::size_t prog, std::size_t vao){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->draw(prog, vao);
             break;
        default: return;
    }
}
void nre::gpu::draw(std::size_t prog, std::size_t vao, std::size_t ibo){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->draw(prog, vao, ibo);
             break;
        default: return;
    }
}

void nre::gpu::draw_instanced(draw_call dcall, int count){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
            if ((dcall.offset == 0) and (dcall.amount == 0)){
                if (dcall.index_buf == 0)
                    static_cast<NRE_GL3_API*>(nre::gpu::api)->draw_instanced(dcall.program, dcall.vertex_layout, count);
                else
                    static_cast<NRE_GL3_API*>(nre::gpu::api)->draw_instanced(dcall.program, dcall.vertex_layout, count, dcall.index_buf);
            }
            else{
                if (dcall.index_buf == 0)
                    //TODO: Error unimplemented function
                    throw 6;
                else
                    //TODO: Error unimplemented function
                    throw 6;
            }
            break;
        default: return;
    }
}

void nre::gpu::draw_instanced(std::size_t prog, std::size_t vao, std::size_t ibo, int count){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->draw_instanced(prog, vao, ibo, count);
             break;
        default: return;
    }
}

void nre::gpu::set_render_mode(RENDERMODE mode){
    switch(nre::gpu::get_api()){
        case GL:
        case GLES:
             static_cast<NRE_GL3_API*>(nre::gpu::api)->setRenderMode(mode);
             break;
        default: return;
    }
}

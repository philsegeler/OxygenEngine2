#include <Renderer/NRE_GPU_API.h>
#include <Renderer/GLES2/NRE_GLES2_API.h>
#include <types/OE_Libs.h>

#define NRE_GLES2_VERTEXL_LAYOUT_OFFSET(i) ((GLvoid*)(i))

using namespace std;

// small utility function to translate the buffer usages to something opengl understands
// This should be different on other APIs
GLenum NRE2GLES2_BufferUse(nre::gpu::BUFFER_USAGE usage){
    GLenum buf_usage;
    switch(usage){
        case nre::gpu::STATIC: buf_usage = GL_STATIC_DRAW; break;
        case nre::gpu::DYNAMIC: buf_usage = GL_DYNAMIC_DRAW; break;
        case nre::gpu::STREAM: buf_usage = GL_STREAM_DRAW; break;
        default: buf_usage = GL_STATIC_DRAW;
    }
    return buf_usage;
}

// get index of a uniform variable in a shader program
std::size_t NRE_GLES2_ProgramData::hasUniform(std::string name){
    size_t index = 0;
    for (auto x: this->uniforms){
        if (x.name == name){
            return index; 
        }
        index++;
    }
    return this->uniforms.size();
}

bool NRE_GLES2_Program::operator< (const NRE_GLES2_Program& other) const{
    if (this->vs < other.vs){
        return true;
    }
    else if (this->vs == other.vs){
        return this->fs < other.fs;
    }
    else {
        return false;
    }
    return false;
}

bool NRE_GLES2_Texture::hasNotChanged(nre::gpu::TEXTURE_TYPE type_in, nre::gpu::TEXTURE_FILTER filter_in, int x_in, int y_in, int mipmaps_in){
    return (this->type == type_in) and (this->filter == filter_in) and (this->x == x_in) and (this->y == y_in) and (this->mipmaps == mipmaps_in);
}

bool NRE_GLES2_RenderBuffer::hasNotChanged(nre::gpu::TEXTURE_TYPE type_in, int x_in, int y_in){
    return (this->type == type_in) and (this->x == x_in) and (this->y == y_in);
}

// ------------------------ API ---------------------- //

std::size_t NRE_GLES2_API::getVAOSize(std::size_t id){
    this->check_vao_id_(id, "getVAOSize");
    return this->vbos[this->vaos[id].layout[0].vertex_buffer].size/this->vaos[id].layout[0].amount;
}


NRE_GLES2_API::NRE_GLES2_API(){
    this->vao_ibos_[0] = 0;
        cout << "[NRE GL API Info] glDebugMessageCallback not available" << endl;
}

NRE_GLES2_API::~NRE_GLES2_API(){
    
}

void NRE_GLES2_API::update(uint32_t x_in, uint32_t y_in){
    
    if (x_in != nre::gpu::x or y_in != nre::gpu::y){
        glViewport(0, 0, x_in, y_in);
        nre::gpu::x = x_in;
        nre::gpu::y = y_in;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthMask(GL_TRUE);
    glStencilMask(0xFF);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    this->active_prog_ = 0;
    this->active_vao_ = 0;
    this->active_vbo_ = 0;
    
    for (auto x: this->vao_ibos_){
        this->vao_ibos_[x.first] = 0;
    }
}

void NRE_GLES2_API::destroy(){
    for (auto x: std::exchange(rbos, {}))
        glDeleteRenderbuffers(1, &x.second.handle);
    for (auto x: std::exchange(vbos, {}))
        glDeleteBuffers(1, &x.second.handle);
    for (auto x: std::exchange(vaos, {}))
        glDeleteVertexArrays(1, &x.second.handle);
    for (auto x: std::exchange(ibos, {}))
        glDeleteBuffers(1, &x.second.handle);
    for (auto x: std::exchange(fbos, {}))
        glDeleteFramebuffers(1, &x.second.handle);
    for (auto x: std::exchange(textures, {}))
        glDeleteTextures(1, &x.second.handle);
    for (auto x: std::exchange(prog_db, {})){
        glDeleteProgram(x.second.handle);
    }
    for (auto x: std::exchange(vs_db, {})){
        glDeleteShader(x.second);
    }
    for (auto x: std::exchange(fs_db, {})){
        glDeleteShader(x.second);
    }
}

std::string NRE_GLES2_API::getRenderingAPI(){
    
        return "OpenGL ES 2";
}

//-------------------Handle errors--------------------------------//

void NRE_GLES2_API::check_rbo_id_(std::size_t id, const std::string& func){
    if(this->rbos.count(id) == 0){
        throw nre::gpu::invalid_render_buffer(id, func);
    }
}

void NRE_GLES2_API::check_vbo_id_(std::size_t id, const std::string& func){
    if(this->vbos.count(id) == 0){
        throw nre::gpu::invalid_vertex_buffer(id, func);
    }
}
void NRE_GLES2_API::check_ibo_id_(std::size_t id, const std::string& func){
    if(this->ibos.count(id) == 0){
        throw nre::gpu::invalid_index_buffer(id, func);
    }
}
    
void NRE_GLES2_API::check_vbo_offset_length_(std::size_t id, std::size_t off_len, const std::string& func){
    if (off_len > this->vbos[id].size){
        throw nre::gpu::invalid_buffer_offset_length(id, off_len, "vertex", func);
    }
}

void NRE_GLES2_API::check_ibo_offset_length_(std::size_t id, std::size_t off_len, const std::string& func){
    if (off_len > this->ibos[id].size){
        throw nre::gpu::invalid_buffer_offset_length(id, off_len, "index", func);
    }
}
    
void NRE_GLES2_API::check_vao_id_(std::size_t id, const std::string& func){
    if(this->vaos.count(id) == 0){
        throw nre::gpu::invalid_vertex_layout(id, func);
    }
}

void NRE_GLES2_API::check_prog_id_(std::size_t id, const std::string& func){
    if(this->progs.count(id) == 0){
        throw nre::gpu::invalid_program_id(id, func);
    }
}

void NRE_GLES2_API::check_prog_complete_(std::size_t id, const std::string& func){
    if (not this->progs[id].setup){
        throw nre::gpu::incomplete_program(id, func);
    }
}

void NRE_GLES2_API::check_prog_uniform_(std::size_t id, const std::string& name, const std::string& func){
    if (this->prog_db[this->progs[id]].hasUniform(name) == this->prog_db[this->progs[id]].uniforms.size()){
        throw nre::gpu::invalid_program_uniform(id, name, func);
    }
}

void NRE_GLES2_API::check_prog_uniform_property_(std::size_t id, const std::string& name, std::size_t length, const std::string& func, bool is_type_problem){
    auto uniform_typ = this->prog_db[this->progs[id]].uniforms[this->prog_db[this->progs[id]].hasUniform(name)].type;
    bool is_vec2 = (uniform_typ == GL_FLOAT_VEC2) and (length == 2);
    bool is_vec3 = (uniform_typ == GL_FLOAT_VEC3) and (length == 3);
    bool is_vec4 = (uniform_typ == GL_FLOAT_VEC4) and (length == 4);
    bool is_mat2 = (uniform_typ == GL_FLOAT_MAT2) and (length == 4);
    bool is_mat3 = (uniform_typ == GL_FLOAT_MAT3) and (length == 9);
    bool is_mat4 = (uniform_typ == GL_FLOAT_MAT4) and (length == 16);
    if (is_vec2 or is_vec3 or is_vec4 or is_mat2 or is_mat3 or is_mat4){
        throw nre::gpu::invalid_uniform_property(id, name, length, func, is_type_problem);
    }
}

void NRE_GLES2_API::check_vao_vbo_(std::size_t id , std::size_t vbo_id, const std::string& func){
    if(this->vbos.count(vbo_id) == 0){
        throw nre::gpu::invalid_vertex_layout_buffer(id, vbo_id, func);
    }
}

void NRE_GLES2_API::check_fbo_id_(std::size_t id, const std::string& func){
    if(this->fbos.count(id) == 0){
        throw nre::gpu::invalid_framebuffer(id, func);
    }
}

void NRE_GLES2_API::check_texture_id_(std::size_t id, const std::string& func){
    if(this->textures.count(id) == 0){
        throw nre::gpu::invalid_texture(id, func);
    }
}

void NRE_GLES2_API::check_draw_range_(std::size_t id , std::size_t length, std::size_t offset, std::size_t count, const std::string& func){
    if( (offset + count) > length){
        throw nre::gpu::invalid_draw_range(id, length, offset, count, func);
    }
}

void NRE_GLES2_API::get_program_all_uniforms_(std::size_t id){
    
    GLint numUniforms=0;
    glGetProgramiv(this->prog_db[this->progs[id]].handle, GL_ACTIVE_UNIFORMS, &numUniforms);
    for(GLint ida=0; ida< numUniforms; ida++){

        GLint name_length=0;
        GLuint idb = ida;
        
#ifndef __EMSCRIPTEN__
        glGetActiveUniformsiv(this->prog_db[this->progs[id]].handle, 1, &idb, GL_UNIFORM_NAME_LENGTH, &name_length);
#else
        
        glGetProgramiv(this->prog_db[this->progs[id]].handle, GL_ACTIVE_UNIFORM_MAX_LENGTH
, &name_length);
        //cout << "uniforms:" << ida << " name length:" << name_length << endl;
#endif
        GLchar uniform_name[name_length];
        GLenum var_enum;
        GLint uniform_size;
        glGetActiveUniform(this->prog_db[this->progs[id]].handle, ida, name_length, &name_length, &uniform_size, &var_enum, &uniform_name[0]);

        string actual_name = string(uniform_name);
        auto uniform_state = NRE_GLES2_ProgramUniformState();
        uniform_state.name = actual_name;
        uniform_state.slot = -1;
        uniform_state.type = var_enum;
        uniform_state.size = uniform_size;
        //cout << uniform_state.name << " " << uniform_state.type << " " << uniform_state.size << endl;
        this->prog_db[this->progs[id]].uniforms.push_back(uniform_state);
    }
}

int NRE_GLES2_API::teximage_internalformat_(nre::gpu::TEXTURE_TYPE type){
    switch (type){

        case nre::gpu::FLOAT:
            return GL_RGB32F;
        case nre::gpu::RGBA:
            return GL_RGBA8;
        case nre::gpu::RGB10_A2:
            return GL_RGB10_A2;
        case nre::gpu::RGBA16F:
            return GL_RGBA16F;
        case nre::gpu::SRGBA:
            return GL_SRGB8_ALPHA8;
        case nre::gpu::RGBA_U16:
            return GL_RGBA16UI;
        case nre::gpu::RGBA_U8:
            return GL_RGBA8UI;
        case nre::gpu::DEPTHSTENCIL:
            return GL_DEPTH24_STENCIL8;
    };
    return GL_RGB;
}

int NRE_GLES2_API::teximage_format_(nre::gpu::TEXTURE_TYPE type){
     switch (type){
        case nre::gpu::FLOAT:
            return GL_RGB32F;
        case nre::gpu::RGBA:
            return GL_RGBA;
        case nre::gpu::RGB10_A2:
            return GL_RGBA;
        case nre::gpu::RGBA16F:
            return GL_RGBA;
        case nre::gpu::SRGBA:
            return GL_RGBA;
        case nre::gpu::RGBA_U16:
            return GL_RGBA_INTEGER;
        case nre::gpu::RGBA_U8:
            return GL_RGBA_INTEGER;
        case nre::gpu::DEPTHSTENCIL:
            return GL_DEPTH_STENCIL;
    };
    return GL_RGB;
}

int NRE_GLES2_API::teximage_type_(nre::gpu::TEXTURE_TYPE type){
     switch (type){
        case nre::gpu::FLOAT:
            return GL_FLOAT;
        case nre::gpu::RGBA:
            return GL_UNSIGNED_BYTE;
        case nre::gpu::RGB10_A2:
            return GL_UNSIGNED_INT_2_10_10_10_REV;
        case nre::gpu::RGBA16F:
            return GL_FLOAT;
        case nre::gpu::SRGBA:
            return GL_UNSIGNED_BYTE;
        case nre::gpu::RGBA_U16:
            return GL_UNSIGNED_SHORT;
        case nre::gpu::RGBA_U8:
            return GL_UNSIGNED_BYTE;
        case nre::gpu::DEPTHSTENCIL:
            return GL_UNSIGNED_INT_24_8;
    };
    return GL_UNSIGNED_BYTE;
}



//---------------------Create Objects-----------------------------//

std::size_t NRE_GLES2_API::newVertexBuffer(){ 
    cur_vbo++;
    this->vbos[cur_vbo] = NRE_GLES2_VertexBuffer();
    glGenBuffers(1, &vbos[cur_vbo].handle);
    return cur_vbo;
     
}
std::size_t NRE_GLES2_API::newVertexLayout(){  
    cur_vao++;
    this->vaos[cur_vao] = NRE_GLES2_VertexArray();
    glGenVertexArrays(1, &vaos[cur_vao].handle);
    this->vao_ibos_[vaos[cur_vao].handle] = 0;
    return cur_vao;
}
std::size_t NRE_GLES2_API::newIndexBuffer(){
    cur_ibo++;
    this->ibos[cur_ibo] = NRE_GLES2_IndexBuffer();
    glGenBuffers(1, &ibos[cur_ibo].handle);
    return cur_ibo;
}
std::size_t NRE_GLES2_API::newProgram(){
    cur_prog++;
    this->progs[cur_prog] = NRE_GLES2_Program();
    return cur_prog;
}

std::size_t NRE_GLES2_API::newFrameBuffer(){ 
    cur_fbo++;
    this->fbos[cur_fbo] = NRE_GLES2_FrameBuffer();
    glGenFramebuffers(1, &fbos[cur_fbo].handle);
    return cur_fbo;
}

std::size_t NRE_GLES2_API::newTexture(){ 
    cur_texture++;
    this->textures[cur_texture] = NRE_GLES2_Texture();
    glGenTextures(1, &textures[cur_texture].handle);
    return cur_texture;
}

std::size_t  NRE_GLES2_API::newRenderBuffer(){
    cur_rbo++;
    this->rbos[cur_rbo] = NRE_GLES2_RenderBuffer();
    glGenRenderbuffers(1, &rbos[cur_rbo].handle);
    return cur_rbo;
}
    
//--------------------Render Buffer -------------------------------//
    
void  NRE_GLES2_API::setRenderBufferType(std::size_t id, nre::gpu::TEXTURE_TYPE a_type, int x, int y){
    this->check_rbo_id_(id, "setRenderBufferType");
    
    if (this->rbos[id].hasNotChanged(a_type, x, y)){
        return;
    }
    
    this->rbos[id].type = a_type;
    this->rbos[id].x = a_type;
    this->rbos[id].y = a_type;
    
    glBindRenderbuffer(GL_RENDERBUFFER, this->rbos[id].handle);
    glRenderbufferStorage(GL_RENDERBUFFER, this->teximage_internalformat_(a_type), x, y);
}

void  NRE_GLES2_API::setFrameBufferRenderBuffer(std::size_t fbo_id, std::size_t rbo_id, int slot){
    this->check_rbo_id_(rbo_id, "setFrameBufferRenderBuffer");
    this->check_fbo_id_(fbo_id, "setFrameBufferRenderBuffer");
    
    glBindFramebuffer(GL_FRAMEBUFFER, this->fbos[fbo_id].handle);
    glBindRenderbuffer(GL_RENDERBUFFER, this->rbos[rbo_id].handle);
    
    if (this->rbos[rbo_id].type != nre::gpu::DEPTHSTENCIL){
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_RENDERBUFFER, this->rbos[rbo_id].handle);  
    }
    else {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rbos[rbo_id].handle);  
    }
    
    if (glGetError() > 0)
        cout << glGetError() << endl;
}

//---------------------Vertex Buffer-----------------------------//

void NRE_GLES2_API::setVertexBufferMemory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage){
    
    this->check_vbo_id_(id, "setVertexBufferMemory");
    
    this->vbos[id].size = memory_size;
    this->vbos[id].usage = buf_usage;
    if (this->active_vbo_ != this->vbos[id].handle){
        glBindBuffer(GL_ARRAY_BUFFER, this->vbos[id].handle);
        this->active_vbo_ = this->vbos[id].handle;
    }
    glBufferData(GL_ARRAY_BUFFER, memory_size*sizeof(float), NULL, NRE2GLES2_BufferUse(buf_usage));
}
void NRE_GLES2_API::setVertexBufferData(std::size_t id, const std::vector<float>& v, std::size_t offset){
    
    this->check_vbo_id_(id, "setVertexBufferData");
    this->check_vbo_offset_length_(id, offset + v.size(), "setVertexBufferData");
    
    if (this->active_vbo_ != this->vbos[id].handle){
        glBindBuffer(GL_ARRAY_BUFFER, this->vbos[id].handle);
        this->active_vbo_ = this->vbos[id].handle;
    }
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLuint>(offset)*sizeof(float), v.size()*sizeof(float), &v[0]);
}

void NRE_GLES2_API::setVertexBufferMemoryData(std::size_t id, const std::vector<float>& v, nre::gpu::BUFFER_USAGE buf_usage){
    
    this->check_vbo_id_(id, "setVertexBufferMemoryData");
    
    this->vbos[id].size = v.size();
    this->vbos[id].usage = buf_usage;
    if (this->active_vbo_ != this->vbos[id].handle){
        glBindBuffer(GL_ARRAY_BUFFER, this->vbos[id].handle);
        this->active_vbo_ = this->vbos[id].handle;
    }
    glBufferData(GL_ARRAY_BUFFER, v.size()*sizeof(float), &v[0], NRE2GLES2_BufferUse(buf_usage));
}

void NRE_GLES2_API::deleteVertexBuffer(std::size_t id){
    this->check_vbo_id_(id, "deleteVertexBuffer");
    
    glDeleteBuffers(1, &this->vbos[id].handle);
    this->active_vbo_ = 0;
    this->vbos.erase(id);
}

//--------------------Index Buffer-------------------------------//

void NRE_GLES2_API::setIndexBufferMemory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage){
    
    this->check_ibo_id_(id, "setIndexBufferMemory");
    
    this->ibos[id].size = memory_size;
    this->ibos[id].usage = buf_usage;
    if (this->vao_ibos_[this->active_vao_] != this->ibos[id].handle){
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos[id].handle);
        this->vao_ibos_[this->active_vao_] = this->ibos[id].handle;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, memory_size*sizeof(uint32_t), NULL, NRE2GLES2_BufferUse(buf_usage));
}

void NRE_GLES2_API::setIndexBufferData(std::size_t id, const std::vector<uint32_t>& v, std::size_t offset){
    
    this->check_ibo_id_(id, "setIndexBufferData");
    this->check_ibo_offset_length_(id, offset + v.size(), "setIndexBufferData");
    
    if (this->vao_ibos_[this->active_vao_] != this->ibos[id].handle){
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos[id].handle);
        this->vao_ibos_[this->active_vao_] = this->ibos[id].handle;
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(offset)*sizeof(uint32_t), v.size()*sizeof(uint32_t), &v[0]);
}

void NRE_GLES2_API::setIndexBufferMemoryData(std::size_t id, const std::vector<uint32_t>& v, nre::gpu::BUFFER_USAGE buf_usage){
    
    this->check_ibo_id_(id, "setIndexBufferMemoryData");
    
    this->ibos[id].size = v.size();
    this->ibos[id].usage = buf_usage;
    if (this->vao_ibos_[this->active_vao_] != this->ibos[id].handle){
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos[id].handle);
        this->vao_ibos_[this->active_vao_] = this->ibos[id].handle;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, v.size()*sizeof(uint32_t), &v[0], NRE2GLES2_BufferUse(buf_usage));
}

void NRE_GLES2_API::deleteIndexBuffer(std::size_t id){
    
    this->check_ibo_id_(id, "deleteIndexBuffer");
    
    glDeleteBuffers(1, &this->ibos[id].handle);
    this->vao_ibos_[this->active_vao_] = 0;
    this->ibos.erase(id);
}

//----------------------Uniform State ----------------//

void NRE_GLES2_API::setProgramTextureSlot(std::size_t id, std::string name, int slot){
    this->check_prog_id_(id, "setProgramTextureSlot");
    this->check_prog_complete_(id, "setProgramUniformData");
    this->check_prog_uniform_(id, name, "setProgramTextureSlot");
    
    if (this->active_prog_ != this->progs[id].handle){
        glUseProgram(this->progs[id].handle);
        this->active_prog_ = this->progs[id].handle;
    }
    auto uniform_type_enum = this->prog_db[this->progs[id]].uniforms[this->prog_db[this->progs[id]].hasUniform(name)].type;
    if ((uniform_type_enum == GL_SAMPLER_2D) or (uniform_type_enum == GL_SAMPLER_2D_SHADOW)){
        glUniform1i(this->prog_db[this->progs[id]].hasUniform(name), slot);
    }
    else {
        cout << "[NRE Warning] No sampler2D uniform named '" << name << "' in program ID: " << id << "." << endl;
        OE_WriteToLog("[NRE Warning] No sampler2D uniform named '" + name + "' in program ID: " + to_string(id) + ".");
    }
}

void NRE_GLES2_API::setProgramUniformData(std::size_t id, std::string name, uint32_t data){
    this->check_prog_id_(id, "setProgramUniformData");
    this->check_prog_complete_(id, "setProgramUniformData");
    this->check_prog_uniform_(id, name, "setProgramUniformData");
    
    if (this->active_prog_ != this->progs[id].handle){
        glUseProgram(this->progs[id].handle);
        this->active_prog_ = this->progs[id].handle;
    }
    auto uniform_type_enum = this->prog_db[this->progs[id]].uniforms[this->prog_db[this->progs[id]].hasUniform(name)].type;
    if (uniform_type_enum == GL_INT){
        glUniform1i(this->prog_db[this->progs[id]].hasUniform(name), data);
    }
    else {
        cout << "[NRE Warning] No integer uniform named '" << name << "' in program ID: " << id << "." << endl;
        OE_WriteToLog("[NRE Warning] No integer uniform named '" + name + "' in program ID: " + to_string(id) + ".");
    }
}

void NRE_GLES2_API::setProgramUniformData(std::size_t id, std::string name, float data){
    this->check_prog_id_(id, "setProgramUniformData");
    this->check_prog_complete_(id, "setProgramUniformData");
    this->check_prog_uniform_(id, name, "setProgramUniformData");
    
    if (this->active_prog_ != this->progs[id].handle){
        glUseProgram(this->progs[id].handle);
        this->active_prog_ = this->progs[id].handle;
    }
    auto uniform_type_enum = this->prog_db[this->progs[id]].uniforms[this->prog_db[this->progs[id]].hasUniform(name)].type;
    if (uniform_type_enum == GL_FLOAT){
        glUniform1f(this->prog_db[this->progs[id]].hasUniform(name), data);
    }
    else {
        cout << "[NRE Warning] No float uniform named '" << name << "' in program ID: " << id << "." << endl;
        OE_WriteToLog("[NRE Warning] No float uniform named '" + name + "' in program ID: " + to_string(id) + ".");
    }
}

void NRE_GLES2_API::setProgramUniformData(std::size_t id, std::string name, const std::vector<float>& data){
    this->check_prog_id_(id, "setProgramUniformData");
    this->check_prog_complete_(id, "setProgramUniformData");
    this->check_prog_uniform_(id, name, "setProgramUniformData");
    this->check_prog_uniform_property_(id, name, data.size(), "setProgramUniformData", false);
    
    if (this->active_prog_ != this->progs[id].handle){
        glUseProgram(this->progs[id].handle);
        this->active_prog_ = this->progs[id].handle;
    }
    auto uniform_type_enum = this->prog_db[this->progs[id]].uniforms[this->prog_db[this->progs[id]].hasUniform(name)].type;
    
    if (uniform_type_enum == GL_FLOAT_VEC2){
        glUniform2f(this->prog_db[this->progs[id]].hasUniform(name), data[0], data[1]);
    }
    else if (uniform_type_enum == GL_FLOAT_VEC3){
        glUniform3f(this->prog_db[this->progs[id]].hasUniform(name), data[0], data[1], data[2]);
    }
    else if ((uniform_type_enum == GL_FLOAT_VEC4) or (uniform_type_enum == GL_FLOAT_MAT2)){
        glUniform4f(this->prog_db[this->progs[id]].hasUniform(name), data[0], data[1], data[2], data[3]);
    }
    else if (uniform_type_enum == GL_FLOAT_MAT3){
        glUniformMatrix3fv(this->prog_db[this->progs[id]].hasUniform(name), 1, false, &data[0]);
    }
    else if (uniform_type_enum == GL_FLOAT_MAT4){
        glUniformMatrix4fv(this->prog_db[this->progs[id]].hasUniform(name), 1, false, &data[0]);
    }
    else{
        throw nre::gpu::invalid_uniform_property(id, name, data.size(), "setProgramUniformData", true);
    }
}

int  NRE_GLES2_API::getProgramUniformSlot(std::size_t id, std::string name){
    this->check_prog_id_(id, "getProgramUniformSlot");
    this->check_prog_complete_(id, "getProgramUniformSlot");
    if (this->prog_db[this->progs[id]].hasUniform(name) != this->prog_db[this->progs[id]].uniforms.size()){
        return this->prog_db[this->progs[id]].uniforms[this->prog_db[this->progs[id]].hasUniform(name)].slot;
    }
    return -2;
}

//---------------------Vertex Layout-----------------------------//

void NRE_GLES2_API::setVertexLayoutFormat(std::size_t id, std::vector<nre::gpu::vertex_layout_input> inputs){ 
    
    this->check_vao_id_(id, "setVertexlayoutFormat");
    
    this->vaos[id].layout = inputs;
    for(size_t x=0; x < inputs.size(); x++){
        this->check_vao_vbo_(id, inputs[x].vertex_buffer, "setVertexLayoutFormat");
    }
}

void NRE_GLES2_API::deleteVertexLayout(std::size_t id){
    this->check_vao_id_(id, "deleteVertexLayout");
    glDeleteVertexArrays(1, &this->vaos[id].handle);
    this->active_vao_ = 0;
    this->vaos.erase(id);
}

//-----------------------Textures and Framebuffers -------------//

void NRE_GLES2_API::setTextureFormat(std::size_t id, nre::gpu::TEXTURE_TYPE type, nre::gpu::TEXTURE_FILTER filter, uint32_t x_in, uint32_t y_in, int mipmap_count){
    this->check_texture_id_(id, "setTextureFormat");
    
    if (this->textures[id].hasNotChanged(type, filter, x_in, y_in, mipmap_count)) return;
    
    this->textures[id].type = type;
    this->textures[id].filter = filter;
    this->textures[id].x = x_in;
    this->textures[id].y = y_in;
    this->textures[id].mipmaps = mipmap_count;
    
    
    glBindTexture(GL_TEXTURE_2D, this->textures[id].handle);
    glTexImage2D(GL_TEXTURE_2D, 0, teximage_internalformat_(type), x_in, y_in, 0, teximage_format_(type), teximage_type_(type), 0);
    if (mipmap_count == 0 and filter == nre::gpu::LINEAR){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    } 
    else if (mipmap_count == 0 and filter == nre::gpu::NEAREST){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
    }
    else{
        //TODO
    }
    if (glGetError() > 0)
        cout << glGetError() << endl;
}

void NRE_GLES2_API::setFrameBufferTexture(std::size_t fbo_id, std::size_t texture_id, int slot){
    this->check_fbo_id_(fbo_id, "setFrameBufferTexture");
    this->check_texture_id_(texture_id, "setFrameBufferTexture");
    assert ((slot >= 0) and (slot < 3));
    
    this->fbos[fbo_id].texture = texture_id;
    
    glBindFramebuffer(GL_FRAMEBUFFER, this->fbos[fbo_id].handle);
    glBindTexture(GL_TEXTURE_2D, this->textures[texture_id].handle);
    
    if (this->textures[texture_id].type != nre::gpu::DEPTHSTENCIL){
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, this->textures[texture_id].handle, 0);  
    }
    else {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, this->textures[texture_id].handle, 0);  
    }
    
    if (glGetError() > 0)
        cout << glGetError() << endl;
    
}

void NRE_GLES2_API::setTextureSlot(std::size_t id, int slot){
    this->check_texture_id_(id, "setTextureSlot");
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, this->textures[id].handle);
}

void NRE_GLES2_API::deleteTexture(std::size_t id){
    this->check_texture_id_(id, "deleteTexture");
    glDeleteTextures(1, &this->textures[id].handle);
    this->textures.erase(id);
}

void NRE_GLES2_API::copyFrameBuffer(std::size_t src, std::size_t target, nre::gpu::FRAMEBUFFER_COPY method){
    this->check_fbo_id_(src, "copyFrameBuffer");
    this->check_texture_id_(this->fbos[src].texture, "copyFrameBuffer");
    
    auto x_tmp = this->textures[this->fbos[src].texture].x;
    auto y_tmp = this->textures[this->fbos[src].texture].y;
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbos[src].handle);
    if (target != 0){
        this->check_fbo_id_(target, "copyFrameBuffer");
        this->check_texture_id_(this->fbos[target].texture, "copyFrameBuffer");
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->fbos[target].handle);
    }
    else {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    if (method == nre::gpu::FBO_COLOR){
    glBlitFramebuffer(0,0, x_tmp, y_tmp, 0, 0, x_tmp, y_tmp, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    } else if (method == nre::gpu::FBO_DEPTHSTENCIL){
        glBlitFramebuffer(0,0, x_tmp, y_tmp, 0, 0, x_tmp, y_tmp, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    }
    else {
        glBlitFramebuffer(0,0, x_tmp, y_tmp, 0, 0, x_tmp, y_tmp, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    }
    //if (glGetError() > 0)
    //    cout << glGetError() << endl;
}

void NRE_GLES2_API::clearFrameBuffer(std::size_t id, nre::gpu::FRAMEBUFFER_COPY clear, float alpha_value){
    this->check_fbo_id_(id, "clearFrameBuffer");
    
    glBindFramebuffer(GL_FRAMEBUFFER, this->fbos[id].handle);
    glDepthMask(GL_TRUE);
    glColorMask(1, 1, 1, 1);
    glClearColor(0.0f, 0.0f, 0.0f, alpha_value);
    
    if (clear == nre::gpu::FBO_ALL){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else if (clear == nre::gpu::FBO_COLOR){
        glClear(GL_COLOR_BUFFER_BIT);
    }
    else if (clear == nre::gpu::FBO_DEPTHSTENCIL){
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else if (clear == nre::gpu::FBO_COLORSTENCIL){
        
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else{
        
    }
    
}

void NRE_GLES2_API::useFrameBuffer(std::size_t id){
    if (id != 0){
        this->check_fbo_id_(id, "useFrameBuffer");
        glBindFramebuffer(GL_FRAMEBUFFER, this->fbos[id].handle);
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void NRE_GLES2_API::deleteFrameBuffer(std::size_t id){
    this->check_fbo_id_(id, "deleteFrameBuffer");
    glDeleteFramebuffers(1, &this->fbos[id].handle);
    this->fbos.erase(id);
}

//---------------------Shader Programs-----------------------------//

void NRE_GLES2_API::setProgramVS(std::size_t id, nre::gpu::vertex_shader vs){
    this->check_prog_id_(id, "setProgramVS");
    
    this->progs[id].vs_setup = false;
    this->progs[id].setup = false;
    this->progs[id].vs = vs;
}
void NRE_GLES2_API::setProgramFS(std::size_t id, nre::gpu::pixel_shader fs){
    this->check_prog_id_(id, "setProgramFS");
    
    this->progs[id].fs_setup = false;
    this->progs[id].setup = false;
    this->progs[id].fs = fs;
}

void NRE_GLES2_API::setProgramVS(std::size_t id, std::string data){
    
    data = data + "";
    this->check_prog_id_(id, "setProgramVS - internal");
    GLuint shader_id;
    if(! this->progs[id].vs_setup)
        shader_id = glCreateShader(GL_VERTEX_SHADER);
    else
        shader_id = this->progs[id].vs_handle;
    
    this->progs[id].vs_setup = true;
    this->progs[id].vs_handle = shader_id;
    this->progs[id].setup = false;
    
    /// compile and attach shader
    const char* c_str = data.c_str();
    
    GLint data_size = data.length();
    glShaderSource(shader_id, 1, &c_str, &data_size);
    glCompileShader(shader_id);

    /// IMPORTANT: get shader compiler info
    int parameters = -1;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &parameters);
    if(parameters != GL_TRUE){

        int max_length = 2048;
        int actual_length = 0;
        char log[2048];
        glGetShaderInfoLog(shader_id, max_length, &actual_length, log);
        
        stringstream ss;
        ss << "[NRE GL Shader Warning] in vertex shader: " << this->progs[id].vs.info() << endl << log << endl;
        cout << ss.str();
        OE_WriteToLog(ss.str());
    }
}

//void setProgramGS(std::size_t, FE_GPU_Shader){}
void NRE_GLES2_API::setProgramFS(std::size_t id, std::string data){
    
    data = data + "";
    this->check_prog_id_(id, "setProgramFS - internal");
    GLuint shader_id;
    if(! this->progs[id].fs_setup)
        shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    else
        shader_id = this->progs[id].fs_handle;
    
    this->progs[id].fs_setup = true;
    this->progs[id].fs_handle = shader_id;
    this->progs[id].setup = false;
    
    /// compile and attach shader
    const char* c_str = data.c_str();
    GLint data_size = data.length();
    glShaderSource(shader_id, 1, &c_str, &data_size);
    glCompileShader(shader_id);

    /// IMPORTANT: get shader compiler info
    int parameters = -1;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &parameters);
    if(parameters != GL_TRUE){

        int max_length = 2048;
        int actual_length = 0;
        char log[2048];
        glGetShaderInfoLog(shader_id, max_length, &actual_length, log);
        
        stringstream ss;
        ss << "[NRE GL Shader Warning] in pixel shader: " << this->progs[id].fs.info() << endl << log << endl;
        cout << ss.str();
        OE_WriteToLog(ss.str());
    }
}

//void setProgramTCS(std::size_t, FE_GPU_Shader){}
//void setProgramTES(std::size_t, FE_GPU_Shader){}

void NRE_GLES2_API::setupProgram(std::size_t id){
    
    this->check_prog_id_(id, "setupProgram");
    
    // ignore if the program is already setup
    if (this->progs[id].setup) return;
    
    // setup vertex shader
    if (!this->progs[id].vs_setup){
        if (this->vs_db.count(this->progs[id].vs) == 0){
            
            // vertex shader does not exist, make a new entry
            this->setProgramVS(id, this->progs[id].vs.gen_shader());
            this->vs_db[this->progs[id].vs] = this->progs[id].vs_handle;
            
        }
        else {
            
            // vertex shader already exists, reuse that
            this->progs[id].vs_handle = this->vs_db[this->progs[id].vs];
            this->progs[id].vs_setup = true;
        }
    }
    
    // setup pixel (fragment) shader
    if (!this->progs[id].fs_setup){
        if (this->fs_db.count(this->progs[id].fs) == 0){
            
            // pixel (fragment) shader does not exist, make a new entry
            this->setProgramFS(id, this->progs[id].fs.gen_shader());
            
            this->fs_db[this->progs[id].fs] = this->progs[id].fs_handle;
            this->progs[id].fs_setup = true;
        }
        else {
            
            // pixel (fragment) shader already exists, reuse that
            this->progs[id].fs_handle = this->fs_db[this->progs[id].fs];
            this->progs[id].fs_setup = true;
        }
    }
    
    // check if program already exists
    if (this->prog_db.count(this->progs[id]) > 0){
        
        this->progs[id].handle = this->prog_db[this->progs[id]].handle;
        this->progs[id].setup = true;
        
        return;
    }
    else {
        this->progs[id].handle = glCreateProgram();
        //this->prog_db[this->progs[id]] = this->progs[id].handle;
        this->prog_db[this->progs[id]] = NRE_GLES2_ProgramData();
        this->prog_db[this->progs[id]].handle = this->progs[id].handle;
    }
    
    // make sure that the vertex and fragment shaders actually compile
    // in case the program does not already exist
    assert (this->progs[id].vs_setup && this->progs[id].fs_setup);
    
    glAttachShader(this->progs[id].handle, this->progs[id].vs_handle);
    glAttachShader(this->progs[id].handle, this->progs[id].fs_handle);

    glLinkProgram(this->progs[id].handle);


    /// IMPORTANT: check if program is linked correctly
    int parameters = -1;
    glGetProgramiv(this->progs[id].handle, GL_LINK_STATUS, &parameters);
    if(GL_TRUE != parameters){

        int max_length = 2048;
        int actual_length = 0;
        char log[2048];
        glGetProgramInfoLog(this->progs[id].handle, max_length, &actual_length, log);
        cout << log << endl;
        OE_WriteToLog(log);
    }

    /// IMPORTANT: make sure program is runnable
    glValidateProgram(this->progs[id].handle);
    parameters = -1;
    glGetProgramiv(this->progs[id].handle, GL_VALIDATE_STATUS, &parameters);
    if(GL_TRUE != parameters){

        int max_length = 2048;
        int actual_length = 0;
        char log[2048];
        glGetProgramInfoLog(this->progs[id].handle, max_length, &actual_length, log);
        
        stringstream ss;
        ss << "[NRE GL Shader Linking Warning] with shaders: " << this->progs[id].vs.info() << " " << this->progs[id].fs.info() << endl << log << endl;
        cout << ss.str();
        OE_WriteToLog(ss.str());

    }
    glUseProgram(this->progs[id].handle);
    this->active_prog_ = this->progs[id].handle;
    
    /// get all active uniform blocks
    this->get_program_all_uniforms_(id);
    
    this->progs[id].setup = true;
    
}

void NRE_GLES2_API::deleteProgram(std::size_t id){
    
    this->check_prog_id_(id, "deleteProgram");
    
    this->active_prog_ = 0;
    this->progs.erase(id);
}

//---------------------Draw calls-----------------------------//

void NRE_GLES2_API::draw(nre::gpu::draw_call dc_info){
    
    this->check_prog_id_(dc_info.program, "draw");
    this->check_vao_id_(dc_info.vertex_layout, "draw");
    
    bool is_ranged_rendering = (dc_info.offset != 0) or (dc_info.amount != 0);
    
    // program
    this->setupProgram(dc_info.program);
    if (this->active_prog_ != this->progs[dc_info.program].handle){
        glUseProgram(this->progs[dc_info.program].handle);
        this->active_prog_ = this->progs[dc_info.program].handle;
    }
    
    // vao
    auto vao_id = dc_info.vertex_layout;
    if (this->active_vao_ != vao_id){
        
        // disable unused vertex attributes if enabled before
        if (this->enabled_vao_attribs > this->vaos[vao_id].layout.size()){
            for (std::size_t i = this->enabled_vao_attribs; i > this->vaos[vao_id].layout.size(); i--){
                glDisableVertexAttribArray(i);
            }
        }
        
        for(size_t x=0; x < this->vaos[vao_id].layout.size(); x++){
            
            this->check_vao_vbo_(vao_id, this->vaos[vao_id].layout[x].vertex_buffer, "draw");
            
            glBindBuffer(GL_ARRAY_BUFFER, this->vbos[this->vaos[vao_id].layout[x].vertex_buffer].handle);
            glVertexAttribPointer(x, this->vaos[vao_id].layout[x].amount, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*this->vaos[vao_id].layout[x].stride, NRE_GLES2_VERTEXL_LAYOUT_OFFSET(sizeof(GLfloat)*static_cast<GLuint>(this->vaos[vao_id].layout[x].offset)));
            glEnableVertexAttribArray(x);
            
        }
        this->enabled_vao_attribs = this->vaos[vao_id].layout.size();
        this->active_vbo_ = 0;
    
        this->active_vao_ = vao_id;
    }
    
    // optional elements handling
    if (dc_info.index_buf != 0){
        
        this->check_ibo_id_(dc_info.index_buf, "draw");
        if (this->vao_ibos_[this->active_vao_] != this->ibos[dc_info.index_buf].handle){
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos[dc_info.index_buf].handle);
            this->vao_ibos_[this->active_vao_] = this->ibos[dc_info.index_buf].handle;
        }
        
        if (is_ranged_rendering){
            glDrawElements(GL_TRIANGLES, dc_info.amount, GL_UNSIGNED_INT, (GLvoid*)((sizeof(uint32_t))*3*dc_info.offset));
        }
        else {
            glDrawElements(GL_TRIANGLES, this->ibos[dc_info.index_buf].size, GL_UNSIGNED_INT, (GLvoid*)NULL);
        }
    }
    else{
        if (is_ranged_rendering){
            this->check_draw_range_(vao_id, this->getVAOSize(vao_id), dc_info.offset, dc_info.amount, "draw (indexed)");
            glDrawArrays(GL_TRIANGLES, dc_info.offset, dc_info.amount);
        }
        else{
            glDrawArrays(GL_TRIANGLES, 0, this->getVAOSize(vao_id));
        }
    }
}

void NRE_GLES2_API::setRenderMode(nre::gpu::RENDERMODE rendermode){
    
    if (rendermode == nre::gpu::Z_PREPASS_BACKFACE){
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST); 
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_BACK); /// cull back face
        glFrontFace (GL_CCW);
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glColorMask(0, 0, 0, 0);
        glDepthMask(GL_TRUE);
    }
    else if (rendermode == nre::gpu::REGULAR_FRONTFACE){
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST); 
        
        glEnable (GL_DEPTH_TEST); // enable depth-testing
        glDepthFunc (GL_LESS);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_TRUE);
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_FRONT); /// cull back face
        glFrontFace (GL_CCW);
    }
    else if (rendermode == nre::gpu::REGULAR_BACKFACE){
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST); 
        
        glEnable (GL_DEPTH_TEST); // enable depth-testing
        glDepthFunc (GL_LESS);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_TRUE);
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_BACK); /// cull back face
        glFrontFace (GL_CCW);
    }
    else if (rendermode == nre::gpu::AFTERPREPASS_BACKFACE){
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST); 
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_FALSE);
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_BACK); /// cull back face
        glFrontFace (GL_CCW);
    }
    else if (rendermode == nre::gpu::REGULAR_BOTH){
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST); 
        
        glEnable (GL_DEPTH_TEST); // enable depth-testing
        glDepthFunc (GL_LESS);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_TRUE);
        
        glDisable(GL_CULL_FACE);
    }
    else if (rendermode == nre::gpu::FULLSCREEN_QUAD){
        glDisable(GL_BLEND);
        
        glDisable(GL_STENCIL_TEST); 
        glDisable(GL_DEPTH_TEST);
        //glDepthFunc(GL_LEQUAL);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_TRUE);
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_BACK); /// cull back face
        glFrontFace (GL_CCW);
    }
    else if (rendermode == nre::gpu::LIGHT_PREPASS){
        glDisable(GL_BLEND);
        
        glEnable(GL_STENCIL_TEST); 
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_REPLACE, GL_ZERO);
        
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glColorMask(0, 0, 0, 0);
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_BACK); /// cull back face
        glFrontFace (GL_CCW);
    }
    else if (rendermode == nre::gpu::LIGHT_AFTERPASS){
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_CONSTANT_COLOR);
        glBlendColor(0.25, 0.25, 0.25, 0.25);
        
        glEnable(GL_STENCIL_TEST); 
        glStencilFunc(GL_EQUAL, 0, 0xFF);
        glStencilMask(0x00);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_GEQUAL);
        glColorMask(1, 1, 1, 1);
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_FRONT); /// cull front face
        glFrontFace (GL_CCW);
    }
    else if (rendermode == nre::gpu::LIGHT_PREPASS_2){
        glDisable(GL_BLEND);
        
        glEnable(GL_STENCIL_TEST); 
        glStencilFunc(GL_ALWAYS, 2, 0xFF);
        //glStencilFunc(GL_GEQUAL, 2, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_REPLACE, GL_ZERO);
        //glStencilOp(GL_KEEP, GL_INCR, GL_DECR);
        
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glColorMask(0, 0, 0, 0);
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_BACK); /// cull back face
        glFrontFace (GL_CCW);
    }
    else if (rendermode == nre::gpu::LIGHT_AFTERPASS_RG){
        glDisable(GL_BLEND);
        //glBlendEquation(GL_MAX);
        
        glEnable(GL_STENCIL_TEST); 
        glStencilFunc(GL_GREATER, 2, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_GEQUAL);
        glColorMask(1, 1, 0, 0);
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_FRONT); /// cull front face
        glFrontFace (GL_CCW);
        
    }
    else if (rendermode == nre::gpu::LIGHT_AFTERPASS_BA){
        glDisable(GL_BLEND);
        //glBlendEquation(GL_MAX);
        
        glEnable(GL_STENCIL_TEST); 
        glStencilFunc(GL_EQUAL, 0, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);
        
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_GEQUAL);
        glColorMask(0, 0, 1, 1);
        
        glEnable (GL_CULL_FACE);
        glCullFace (GL_FRONT); /// cull front face
        glFrontFace (GL_CCW);
    }
    else {
        // TODO
    }
    
}



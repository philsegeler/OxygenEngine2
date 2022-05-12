#include <OE/Renderer/GL3/api_gl3.h>
#include <OE/types/libs_oe.h>

#define NRE_GL3_VERTEXL_LAYOUT_OFFSET(i) ((GLvoid*)(i))

using namespace std;

// OpenGL debug function
void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                     const GLchar* message, const void* userParam) {


    stringstream ss;
    ss << "[NRE GL API debug callback START]" << endl;
    ss << "message: " << message << endl;
    ss << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        ss << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        ss << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        ss << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        ss << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        ss << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        ss << "OTHER";
        break;
    }
    ss << endl;

    ss << "id: " << id << endl;
    ss << "severity: ";
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        ss << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        ss << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        ss << "HIGH";
        break;
    }
    ss << endl;
    ss << "[NRE GL API debug callback END]" << endl;
    cout << ss.str();
    OE_WriteToLog(ss.str());
}


// small utility function to translate the buffer usages to something opengl understands
// This should be different on other APIs
GLenum nre::gl3::buffer_use(nre::gpu::BUFFER_USAGE usage) {
    GLenum buf_usage;
    switch (usage) {
    case nre::gpu::STATIC:
        buf_usage = GL_STATIC_DRAW;
        break;
    case nre::gpu::DYNAMIC:
        buf_usage = GL_DYNAMIC_DRAW;
        break;
    case nre::gpu::STREAM:
        buf_usage = GL_STREAM_DRAW;
        break;
    default:
        buf_usage = GL_STATIC_DRAW;
    }
    return buf_usage;
}

// get index of a uniform block variable in a shader program
bool nre::gl3::program_data_t::has_uniform_block(std::string_view name) {
    return this->uniform_blocks.contains(name);
}

// get index of a uniform variable in a shader program
bool nre::gl3::program_data_t::has_uniform(std::string_view name) {
    return this->uniforms.contains(name);
}

bool nre::gl3::program_t::operator==(const nre::gl3::program_t& other) const {
    return std::tie(this->vs, this->fs) == std::tie(other.vs, other.fs);
}
size_t nre::gl3::program_t::gen_hash() const {
    return this->vs.gen_hash() + this->fs.gen_hash();
}

bool nre::gl3::texture_t::has_not_changed(nre::gpu::TEXTURE_TYPE type_in, nre::gpu::TEXTURE_FILTER filter_in, int x_in,
                                          int y_in, int mipmaps_in) {
    return (this->type == type_in) and (this->filter == filter_in) and (this->x == x_in) and (this->y == y_in) and
           (this->mipmaps == mipmaps_in);
}

bool nre::gl3::renderbuffer_t::has_not_changed(nre::gpu::TEXTURE_TYPE type_in, int x_in, int y_in) {
    return (this->type == type_in) and (this->x == x_in) and (this->y == y_in);
}

// ------------------------ API ---------------------- //

std::size_t nre::gl3::api_t::get_vao_size(std::size_t id) {
    this->check_vao_id_(id, "get_vao_size");
    return this->vbos_[this->vaos_[id].layout[0].vertex_buffer].size / this->vaos_[id].layout[0].stride;
}


nre::gl3::api_t::api_t(nre::gpu::info_struct& backend_info) {
    this->vao_ibos_[0] = 0;
    major_             = backend_info.major;
    minor_             = backend_info.minor;
    backend_           = backend_info.underlying_api;

#ifndef OE_PLATFORM_WEB
    if (glDebugMessageCallback) {
        cout << "[NRE GL API Info] Register OpenGL debug callback " << endl;
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglCallbackFunction, nullptr);
        GLuint unusedIds = 0;
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
    }
    else
#endif
        cout << "[NRE GL API Info] glDebugMessageCallback not available" << endl;
}

nre::gl3::api_t::~api_t() {
}

void nre::gl3::api_t::update(uint32_t x_in, uint32_t y_in, bool sanity_checks) {

    sanity_checks_ = sanity_checks;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (x_in != x_ or y_in != y_) {
        glViewport(0, 0, x_in, y_in);
        x_ = x_in;
        y_ = y_in;
    }

    glDepthMask(GL_TRUE);
    glStencilMask(0xFF);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    this->active_prog_ = 0;
    this->active_vao_  = 0;
    this->active_vbo_  = 0;
    this->active_ubo_  = 0;

    for (auto x : this->vao_ibos_) {
        this->vao_ibos_[x.first] = 0;
    }
}

void nre::gl3::api_t::destroy() {
    for (auto x : std::exchange(rbos_, {}))
        glDeleteRenderbuffers(1, &x.second.handle);
    for (auto x : std::exchange(vbos_, {}))
        glDeleteBuffers(1, &x.second.handle);
    for (auto x : std::exchange(vaos_, {}))
        glDeleteVertexArrays(1, &x.second.handle);
    for (auto x : std::exchange(ibos_, {}))
        glDeleteBuffers(1, &x.second.handle);
    for (auto x : std::exchange(ubos_, {}))
        glDeleteBuffers(1, &x.second.handle);
    for (auto x : std::exchange(fbos_, {}))
        glDeleteFramebuffers(1, &x.second.handle);
    for (auto x : std::exchange(textures_, {}))
        glDeleteTextures(1, &x.second.handle);
    for (auto x : std::exchange(prog_db_, {})) {
        glDeleteProgram(x.second.handle);
    }
    for (auto x : std::exchange(vs_db_, {})) {
        glDeleteShader(x.second);
    }
    for (auto x : std::exchange(fs_db_, {})) {
        glDeleteShader(x.second);
    }
}

std::string nre::gl3::api_t::get_rendering_api() {
    if (nre::gpu::get_api() == nre::gpu::GLES)
        return "OpenGL ES 3.1+/WebGL 2.0+";
    else if (nre::gpu::get_api() == nre::gpu::GL)
        return "OpenGL 3.3+";
    else {
        return "Unknown";
    }
}

//-------------------Handle errors--------------------------------//

void nre::gl3::api_t::check_rbo_id_(std::size_t id, std::string_view func) {
    if (this->rbos_.count(id) == 0) {
        throw nre::gpu::invalid_render_buffer(id, func);
    }
}

void nre::gl3::api_t::check_vbo_id_(std::size_t id, std::string_view func) {
    if (this->vbos_.count(id) == 0) {
        throw nre::gpu::invalid_vertex_buffer(id, func);
    }
}
void nre::gl3::api_t::check_ubo_id_(std::size_t id, std::string_view func) {
    if (this->ubos_.count(id) == 0) {
        throw nre::gpu::invalid_uniform_buffer(id, func);
    }
}
void nre::gl3::api_t::check_ibo_id_(std::size_t id, std::string_view func) {
    if (this->ibos_.count(id) == 0) {
        throw nre::gpu::invalid_index_buffer(id, func);
    }
}

void nre::gl3::api_t::check_vbo_offset_length_(std::size_t id, std::size_t off_len, std::string_view func) {
    if (off_len > this->vbos_[id].size) {
        throw nre::gpu::invalid_buffer_offset_length(id, off_len, "vertex", func);
    }
}

void nre::gl3::api_t::check_ubo_offset_length_(std::size_t id, std::size_t off_len, std::string_view func) {
    if (off_len > this->ubos_[id].size) {
        throw nre::gpu::invalid_buffer_offset_length(id, off_len, "uniform", func);
    }
}

void nre::gl3::api_t::check_ibo_offset_length_(std::size_t id, std::size_t off_len, std::string_view func) {
    if (off_len > this->ibos_[id].size) {
        throw nre::gpu::invalid_buffer_offset_length(id, off_len, "index", func);
    }
}

void nre::gl3::api_t::check_vao_id_(std::size_t id, std::string_view func) {
    if (this->vaos_.count(id) == 0) {
        throw nre::gpu::invalid_vertex_layout(id, func);
    }
}

void nre::gl3::api_t::check_prog_id_(std::size_t id, std::string_view func) {
    if (not this->progs_.contains(id)) {
        throw nre::gpu::invalid_program_id(id, func);
    }
}

void nre::gl3::api_t::check_prog_complete_(std::size_t id, std::string_view func) {
    if (not this->progs_[id].setup) {
        throw nre::gpu::incomplete_program(id, func);
    }
}


void nre::gl3::api_t::check_prog_uniform_block_(std::size_t id, std::string_view name, std::string_view func) {
    if (not this->prog_db_[this->progs_[id]].has_uniform_block(name)) {
        throw nre::gpu::invalid_program_uniform_block(id, name, func);
    }
}

void nre::gl3::api_t::check_prog_uniform_(std::size_t id, std::string_view name, std::string_view func) {
    if (not this->prog_db_[this->progs_[id]].has_uniform(name)) {
        throw nre::gpu::invalid_program_uniform(id, name, func);
    }
}

void nre::gl3::api_t::check_prog_uniform_property_(std::size_t id, std::string_view name, std::size_t length,
                                                   std::string_view func, bool is_type_problem) {
    auto uniform_typ = this->prog_db_[this->progs_[id]].uniforms[std::string(name)].type;
    bool is_vec2     = (uniform_typ == GL_FLOAT_VEC2) and (length >= 2);
    bool is_vec3     = (uniform_typ == GL_FLOAT_VEC3) and (length >= 3);
    bool is_vec4     = (uniform_typ == GL_FLOAT_VEC4) and (length >= 4);
    bool is_mat2     = (uniform_typ == GL_FLOAT_MAT2) and (length >= 4);
    bool is_mat3     = (uniform_typ == GL_FLOAT_MAT3) and (length >= 9);
    bool is_mat4     = (uniform_typ == GL_FLOAT_MAT4) and (length >= 16);
    if (not(is_vec2 or is_vec3 or is_vec4 or is_mat2 or is_mat3 or is_mat4)) {
        throw nre::gpu::invalid_uniform_property(id, name, length, func, is_type_problem);
    }
}

void nre::gl3::api_t::check_vao_vbo_(std::size_t id, std::size_t vbo_id, std::string_view func) {
    if (this->vbos_.count(vbo_id) == 0) {
        throw nre::gpu::invalid_vertex_layout_buffer(id, vbo_id, func);
    }
}

void nre::gl3::api_t::check_fbo_id_(std::size_t id, std::string_view func) {
    if (this->fbos_.count(id) == 0) {
        throw nre::gpu::invalid_framebuffer(id, func);
    }
}

void nre::gl3::api_t::check_texture_id_(std::size_t id, std::string_view func) {
    if (this->textures_.count(id) == 0) {
        throw nre::gpu::invalid_texture(id, func);
    }
}

void nre::gl3::api_t::check_draw_range_(std::size_t id, std::size_t length, std::size_t offset, std::size_t count,
                                        std::string_view func) {
    if ((offset + count) > length) {
        throw nre::gpu::invalid_draw_range(id, length, offset, count, func);
    }
}

void nre::gl3::api_t::get_program_all_uniforms_(std::size_t id) {

    /// get all active uniform blocks (again)
    GLint numBlocks = 0;
    glGetProgramiv(this->prog_db_[this->progs_[id]].handle, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);
    for (int ida = 0; ida < numBlocks; ida++) {

        GLint name_length = 0;

        glGetActiveUniformBlockiv(this->prog_db_[this->progs_[id]].handle, ida, GL_UNIFORM_BLOCK_NAME_LENGTH, &name_length);

        GLchar ubo_name[name_length];
        glGetActiveUniformBlockName(this->prog_db_[this->progs_[id]].handle, ida, name_length, NULL, &ubo_name[0]);

        string actual_name                                                  = string(ubo_name);
        auto   ubo_state                                                    = nre::gl3::program_uniform_state_t();
        ubo_state.slot                                                      = -1;
        this->prog_db_[this->progs_[id]].uniform_blocks[actual_name]        = ubo_state;
        this->prog_db_[this->progs_[id]].uniform_block_indices[actual_name] = ida;
    }

    GLint numUniforms = 0;
    glGetProgramiv(this->prog_db_[this->progs_[id]].handle, GL_ACTIVE_UNIFORMS, &numUniforms);
    for (GLint ida = 0; ida < numUniforms; ida++) {

        GLint name_length = 0;


#ifndef __EMSCRIPTEN__
        GLuint idb = ida;
        glGetActiveUniformsiv(this->prog_db_[this->progs_[id]].handle, 1, &idb, GL_UNIFORM_NAME_LENGTH, &name_length);
#else

        glGetProgramiv(this->prog_db_[this->progs_[id]].handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_length);
        // cout << "uniforms:" << ida << " name length:" << name_length << endl;
#endif
        GLchar uniform_name[name_length];
        GLenum var_enum;
        GLint  uniform_size;
        glGetActiveUniform(this->prog_db_[this->progs_[id]].handle, ida, name_length, &name_length, &uniform_size, &var_enum,
                           &uniform_name[0]);

        string actual_name   = string(uniform_name);
        auto   uniform_state = nre::gl3::program_uniform_state_t();
        uniform_state.slot   = -1;
        uniform_state.type   = var_enum;
        uniform_state.size   = uniform_size;
        // cout << uniform_state.name << " " << uniform_state.type << " " << uniform_state.size << endl;
        this->prog_db_[this->progs_[id]].uniforms[actual_name] = uniform_state;
    }
}

int nre::gl3::api_t::teximage_internalformat_(nre::gpu::TEXTURE_TYPE type) {
    switch (type) {

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

int nre::gl3::api_t::teximage_format_(nre::gpu::TEXTURE_TYPE type) {
    switch (type) {
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

int nre::gl3::api_t::teximage_type_(nre::gpu::TEXTURE_TYPE type) {
    switch (type) {
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

std::size_t nre::gl3::api_t::new_vertex_buffer() {
    cur_vbo_++;
    this->vbos_[cur_vbo_] = nre::gl3::vertex_buffer_t();
    glGenBuffers(1, &vbos_[cur_vbo_].handle);
    return cur_vbo_;
}
std::size_t nre::gl3::api_t::new_vertex_layout() {
    cur_vao_++;
    this->vaos_[cur_vao_] = nre::gl3::vertex_layout_t();
    glGenVertexArrays(1, &vaos_[cur_vao_].handle);
    this->vao_ibos_[vaos_[cur_vao_].handle] = 0;
    return cur_vao_;
}
std::size_t nre::gl3::api_t::new_index_buffer() {
    cur_ibo_++;
    this->ibos_[cur_ibo_] = nre::gl3::index_buffer_t();
    glGenBuffers(1, &ibos_[cur_ibo_].handle);
    return cur_ibo_;
}
std::size_t nre::gl3::api_t::new_program() {
    cur_prog_++;
    this->progs_[cur_prog_] = nre::gl3::program_t();
    return cur_prog_;
}

std::size_t nre::gl3::api_t::new_uniform_buffer() {
    cur_ubo_++;
    this->ubos_[cur_ubo_] = nre::gl3::uniform_buffer_t();
    glGenBuffers(1, &ubos_[cur_ubo_].handle);
    return cur_ubo_;
}

std::size_t nre::gl3::api_t::new_framebuffer() {
    cur_fbo_++;
    this->fbos_[cur_fbo_] = nre::gl3::framebuffer_t();
    glGenFramebuffers(1, &fbos_[cur_fbo_].handle);
    return cur_fbo_;
}

std::size_t nre::gl3::api_t::new_texture() {
    cur_texture_++;
    this->textures_[cur_texture_] = nre::gl3::texture_t();
    glGenTextures(1, &textures_[cur_texture_].handle);
    return cur_texture_;
}

std::size_t nre::gl3::api_t::new_renderbuffer() {
    cur_rbo_++;
    this->rbos_[cur_rbo_] = nre::gl3::renderbuffer_t();
    glGenRenderbuffers(1, &rbos_[cur_rbo_].handle);
    return cur_rbo_;
}

//--------------------Render Buffer -------------------------------//

void nre::gl3::api_t::set_renderbuffer_textype(std::size_t id, nre::gpu::TEXTURE_TYPE a_type, int x, int y) {
    this->check_rbo_id_(id, "set_renderbuffer_textype");

    if (this->rbos_[id].has_not_changed(a_type, x, y)) {
        return;
    }

    this->rbos_[id].type = a_type;
    this->rbos_[id].x    = a_type;
    this->rbos_[id].y    = a_type;

    glBindRenderbuffer(GL_RENDERBUFFER, this->rbos_[id].handle);
    glRenderbufferStorage(GL_RENDERBUFFER, this->teximage_internalformat_(a_type), x, y);
}

void nre::gl3::api_t::set_framebuffer_renderbuffer(std::size_t fbo_id, std::size_t rbo_id, int slot) {
    this->check_rbo_id_(rbo_id, "set_framebuffer_renderbuffer");
    this->check_fbo_id_(fbo_id, "set_framebuffer_renderbuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, this->fbos_[fbo_id].handle);
    glBindRenderbuffer(GL_RENDERBUFFER, this->rbos_[rbo_id].handle);

    if (this->rbos_[rbo_id].type != nre::gpu::DEPTHSTENCIL) {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_RENDERBUFFER, this->rbos_[rbo_id].handle);
    }
    else {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rbos_[rbo_id].handle);
    }

    if (glGetError() > 0) cout << glGetError() << endl;
}

//---------------------Vertex Buffer-----------------------------//

void nre::gl3::api_t::set_vertex_buffer_memory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage) {

    this->check_vbo_id_(id, "set_vertex_buffer_memory");

    this->vbos_[id].size  = memory_size;
    this->vbos_[id].usage = buf_usage;
    if (this->active_vbo_ != this->vbos_[id].handle) {
        glBindBuffer(GL_ARRAY_BUFFER, this->vbos_[id].handle);
        this->active_vbo_ = this->vbos_[id].handle;
    }
    glBufferData(GL_ARRAY_BUFFER, memory_size * sizeof(float), NULL, nre::gl3::buffer_use(buf_usage));
}
void nre::gl3::api_t::set_vertex_buffer_data(std::size_t id, const std::vector<float>& v, std::size_t offset) {

    this->check_vbo_id_(id, "set_vertex_buffer_data");
    this->check_vbo_offset_length_(id, offset + v.size(), "set_vertex_buffer_data");

    if (this->active_vbo_ != this->vbos_[id].handle) {
        glBindBuffer(GL_ARRAY_BUFFER, this->vbos_[id].handle);
        this->active_vbo_ = this->vbos_[id].handle;
    }
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLuint>(offset) * sizeof(float), v.size() * sizeof(float), &v[0]);
}

void nre::gl3::api_t::set_vertex_buffer_memory_data(std::size_t id, const std::vector<float>& v,
                                                    nre::gpu::BUFFER_USAGE buf_usage) {

    this->check_vbo_id_(id, "set_vertex_buffer_memory_data");

    this->vbos_[id].size  = v.size();
    this->vbos_[id].usage = buf_usage;
    if (this->active_vbo_ != this->vbos_[id].handle) {
        glBindBuffer(GL_ARRAY_BUFFER, this->vbos_[id].handle);
        this->active_vbo_ = this->vbos_[id].handle;
    }
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), &v[0], nre::gl3::buffer_use(buf_usage));
}

void nre::gl3::api_t::delete_vertex_buffer(std::size_t id) {
    this->check_vbo_id_(id, "delete_vertex_buffer");

    glDeleteBuffers(1, &this->vbos_[id].handle);
    this->active_vbo_ = 0;
    this->vbos_.erase(id);
}

//--------------------Index Buffer-------------------------------//

void nre::gl3::api_t::set_index_buffer_memory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage) {

    this->check_ibo_id_(id, "set_index_buffer_memory");

    this->ibos_[id].size  = memory_size;
    this->ibos_[id].usage = buf_usage;
    if (this->vao_ibos_[this->active_vao_] != this->ibos_[id].handle) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos_[id].handle);
        this->vao_ibos_[this->active_vao_] = this->ibos_[id].handle;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, memory_size * sizeof(uint32_t), NULL, nre::gl3::buffer_use(buf_usage));
}

void nre::gl3::api_t::set_index_buffer_data(std::size_t id, const std::vector<uint32_t>& v, std::size_t offset) {

    this->check_ibo_id_(id, "set_index_buffer_data");
    this->check_ibo_offset_length_(id, offset + v.size(), "set_index_buffer_data");

    if (this->vao_ibos_[this->active_vao_] != this->ibos_[id].handle) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos_[id].handle);
        this->vao_ibos_[this->active_vao_] = this->ibos_[id].handle;
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(offset) * sizeof(uint32_t), v.size() * sizeof(uint32_t),
                    &v[0]);
}

void nre::gl3::api_t::set_index_buffer_memory_data(std::size_t id, const std::vector<uint32_t>& v,
                                                   nre::gpu::BUFFER_USAGE buf_usage) {

    this->check_ibo_id_(id, "set_index_buffer_memory_data");

    this->ibos_[id].size  = v.size();
    this->ibos_[id].usage = buf_usage;
    if (this->vao_ibos_[this->active_vao_] != this->ibos_[id].handle) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos_[id].handle);
        this->vao_ibos_[this->active_vao_] = this->ibos_[id].handle;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, v.size() * sizeof(uint32_t), &v[0], nre::gl3::buffer_use(buf_usage));
}

void nre::gl3::api_t::delete_index_buffer(std::size_t id) {

    this->check_ibo_id_(id, "delete_index_buffer");

    glDeleteBuffers(1, &this->ibos_[id].handle);
    this->vao_ibos_[this->active_vao_] = 0;
    this->ibos_.erase(id);
}

//--------------------Uniform Buffer-----------------------------//

void nre::gl3::api_t::set_uniform_buffer_memory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage) {

    this->check_ubo_id_(id, "set_uniform_buffer_memory");

    this->ubos_[id].size  = memory_size;
    this->ubos_[id].usage = buf_usage;
    if (this->active_ubo_ != this->ubos_[id].handle) {
        glBindBuffer(GL_UNIFORM_BUFFER, this->ubos_[id].handle);
        this->active_ubo_ = this->ubos_[id].handle;
    }
    glBufferData(GL_UNIFORM_BUFFER, memory_size * sizeof(float), NULL, nre::gl3::buffer_use(buf_usage));
}

void nre::gl3::api_t::set_uniform_buffer_data(std::size_t id, const std::vector<float>& v, std::size_t offset) {

    this->check_ubo_id_(id, "set_uniform_buffer_data");
    this->check_ubo_offset_length_(id, offset + v.size(), "set_uniform_buffer_data");

    if (this->active_ubo_ != this->ubos_[id].handle) {
        glBindBuffer(GL_UNIFORM_BUFFER, this->ubos_[id].handle);
        this->active_ubo_ = this->ubos_[id].handle;
    }
    glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLuint>(offset) * sizeof(float), v.size() * sizeof(float), &v[0]);
}

void nre::gl3::api_t::set_uniform_buffer_data(std::size_t id, const std::vector<uint32_t>& v, std::size_t offset) {

    this->check_ubo_id_(id, "set_uniform_buffer_data");
    this->check_ubo_offset_length_(id, offset + v.size(), "set_uniform_buffer_data");

    if (this->active_ubo_ != this->ubos_[id].handle) {
        glBindBuffer(GL_UNIFORM_BUFFER, this->ubos_[id].handle);
        this->active_ubo_ = this->ubos_[id].handle;
    }
    glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLuint>(offset) * sizeof(uint32_t), v.size() * sizeof(uint32_t), &v[0]);
}

//----------------------Uniform State ----------------//

void nre::gl3::api_t::set_program_texture_slot(std::size_t id, std::string_view name, int slot) {
    this->check_prog_id_(id, "set_program_texture_slot");
    this->check_prog_complete_(id, "set_program_texture_slot");
    this->check_prog_uniform_(id, name, "set_program_texture_slot");


    if (this->active_prog_ != this->progs_[id].handle) {
        glUseProgram(this->progs_[id].handle);
        this->active_prog_ = this->progs_[id].handle;
    }
    auto uniform_type_enum = this->prog_db_[this->progs_[id]].uniforms[std::string(name)].type;
    if ((uniform_type_enum == GL_SAMPLER_2D) or (uniform_type_enum == GL_SAMPLER_2D_SHADOW)) {
        glUniform1i(this->prog_db_[this->progs_[id]].uniforms[std::string(name)].slot, slot);
    }
    else {
        cout << "[NRE Warning] No sampler2D uniform named '" << name << "' in program ID: " << id << "." << endl;
        OE_WriteToLog("[NRE Warning] No sampler2D uniform named '" + std::string(name) + "' in program ID: " + to_string(id) +
                      ".");
    }
}
void nre::gl3::api_t::set_program_uniform_data(std::size_t id, std::string_view name, uint32_t data) {
    this->check_prog_id_(id, "set_program_uniform_data");
    this->check_prog_complete_(id, "set_program_uniform_data");
    this->check_prog_uniform_(id, name, "set_program_uniform_data");
    // TODO
}
void nre::gl3::api_t::set_program_uniform_data(std::size_t id, std::string_view name, std::vector<uint32_t> data) {
    this->check_prog_id_(id, "set_program_uniform_data");
    this->check_prog_complete_(id, "set_program_uniform_data");
    this->check_prog_uniform_(id, name, "set_program_uniform_data");
    // TODO
}

int nre::gl3::api_t::get_program_uniform_slot(std::size_t id, std::string_view name) {
    this->check_prog_id_(id, "get_program_uniform_slot");
    this->check_prog_complete_(id, "get_program_uniform_slot");
    if (this->prog_db_[this->progs_[id]].has_uniform(name)) {
        return this->prog_db_[this->progs_[id]].uniforms[std::string(name)].slot;
    }
    return -2;
}

void nre::gl3::api_t::set_program_uniform_block_slot(std::size_t id, std::string_view name, int slot) {

    this->check_prog_id_(id, "set_program_uniform_block_slot");
    this->check_prog_complete_(id, "set_program_uniform_block_slot");
    this->check_prog_uniform_block_(id, name, "set_program_uniform_block_slot");

    this->prog_db_[this->progs_[id]].uniform_blocks[std::string(name)].slot = slot;

    glUniformBlockBinding(this->progs_[id].handle, this->prog_db_[this->progs_[id]].uniform_block_indices[std::string(name)],
                          slot);
}

void nre::gl3::api_t::set_uniform_block_state(std::size_t id, std::size_t program, int slot, std::size_t offset,
                                              std::size_t length) {

    this->check_ubo_id_(id, "set_uniform_block_state");
    this->check_ubo_offset_length_(id, offset + length, "set_uniform_block_state");
    this->check_prog_id_(program, "set_uniform_block_state");

    if (length == 0)
        glBindBufferBase(GL_UNIFORM_BUFFER, slot, this->ubos_[id].handle);
    else
        glBindBufferRange(GL_UNIFORM_BUFFER, slot, this->ubos_[id].handle, static_cast<GLuint>(offset),
                          static_cast<GLuint>(length));
}

int nre::gl3::api_t::get_program_uniform_block_slot(std::size_t id, std::string_view name) {
    this->check_prog_id_(id, "get_program_uniform_block_slot");
    this->check_prog_complete_(id, "get_program_uniform_block_slot");
    if (this->prog_db_[this->progs_[id]].has_uniform_block(name)) {
        return this->prog_db_[this->progs_[id]].uniform_blocks[std::string(name)].slot;
    }
    return -2;
}

void nre::gl3::api_t::delete_uniform_buffer(std::size_t id) {

    this->check_ubo_id_(id, "delete_uniform_buffer");

    glDeleteBuffers(1, &this->ubos_[id].handle);
    this->active_ubo_ = 0;
    this->ubos_.erase(id);
}

//---------------------Vertex Layout-----------------------------//

void nre::gl3::api_t::set_vertex_layout_format(std::size_t id, std::vector<nre::gpu::vertex_layout_input> inputs) {

    this->check_vao_id_(id, "setVertexlayout_format");

    this->vaos_[id].layout = inputs;
    if (this->active_vao_ != this->vaos_[id].handle) {
        glBindVertexArray(this->vaos_[id].handle);
        this->active_vao_ = this->vaos_[id].handle;
    }
    for (size_t x = 0; x < inputs.size(); x++) {

        this->check_vao_vbo_(id, inputs[x].vertex_buffer, "set_vertex_layout_format");

        glBindBuffer(GL_ARRAY_BUFFER, this->vbos_[inputs[x].vertex_buffer].handle);
        glVertexAttribPointer(x, inputs[x].amount, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * inputs[x].stride,
                              NRE_GL3_VERTEXL_LAYOUT_OFFSET(sizeof(GLfloat) * static_cast<GLuint>(inputs[x].offset)));
        glEnableVertexAttribArray(x);
    }
    this->active_vbo_ = 0;
}

void nre::gl3::api_t::delete_vertex_layout(std::size_t id) {
    this->check_vao_id_(id, "delete_vertex_layout");
    glDeleteVertexArrays(1, &this->vaos_[id].handle);
    this->active_vao_ = 0;
    this->vaos_.erase(id);
}

//-----------------------Textures and Framebuffers -------------//

void nre::gl3::api_t::set_texture_format(std::size_t id, nre::gpu::TEXTURE_TYPE type, nre::gpu::TEXTURE_FILTER filter,
                                         uint32_t x_in, uint32_t y_in, int mipmap_count) {
    this->check_texture_id_(id, "set_texture_format");

    if (this->textures_[id].has_not_changed(type, filter, x_in, y_in, mipmap_count)) return;

    this->textures_[id].type    = type;
    this->textures_[id].filter  = filter;
    this->textures_[id].x       = x_in;
    this->textures_[id].y       = y_in;
    this->textures_[id].mipmaps = mipmap_count;


    glBindTexture(GL_TEXTURE_2D, this->textures_[id].handle);
    glTexImage2D(GL_TEXTURE_2D, 0, teximage_internalformat_(type), x_in, y_in, 0, teximage_format_(type), teximage_type_(type),
                 0);
    if (mipmap_count == 0 and filter == nre::gpu::LINEAR) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else if (mipmap_count == 0 and filter == nre::gpu::NEAREST) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else {
        // TODO
    }
    if (glGetError() > 0) cout << glGetError() << endl;
}

void nre::gl3::api_t::set_framebuffer_texture(std::size_t fbo_id, std::size_t texture_id, int slot) {
    this->check_fbo_id_(fbo_id, "set_framebuffer_texture");
    this->check_texture_id_(texture_id, "set_framebuffer_texture");
    assert((slot >= 0) and (slot < 3));

    this->fbos_[fbo_id].texture = texture_id;

    glBindFramebuffer(GL_FRAMEBUFFER, this->fbos_[fbo_id].handle);
    glBindTexture(GL_TEXTURE_2D, this->textures_[texture_id].handle);

    if (this->textures_[texture_id].type != nre::gpu::DEPTHSTENCIL) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, this->textures_[texture_id].handle,
                               0);
    }
    else {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, this->textures_[texture_id].handle,
                               0);
    }

    if (glGetError() > 0) cout << glGetError() << endl;
}

void nre::gl3::api_t::set_texture_slot(std::size_t id, int slot) {
    this->check_texture_id_(id, "set_texture_slot");
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, this->textures_[id].handle);
}

void nre::gl3::api_t::delete_texture(std::size_t id) {
    this->check_texture_id_(id, "delete_texture");
    glDeleteTextures(1, &this->textures_[id].handle);
    this->textures_.erase(id);
}

void nre::gl3::api_t::copy_framebuffer(std::size_t src, std::size_t target, nre::gpu::FRAMEBUFFER_COPY method) {
    this->check_fbo_id_(src, "copy_framebuffer");
    this->check_texture_id_(this->fbos_[src].texture, "copy_framebuffer");

    auto x_tmp = this->textures_[this->fbos_[src].texture].x;
    auto y_tmp = this->textures_[this->fbos_[src].texture].y;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbos_[src].handle);
    if (target != 0) {
        this->check_fbo_id_(target, "copy_framebuffer");
        this->check_texture_id_(this->fbos_[target].texture, "copy_framebuffer");
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->fbos_[target].handle);
    }
    else {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    if (method == nre::gpu::FBO_COLOR) {
        glBlitFramebuffer(0, 0, x_tmp, y_tmp, 0, 0, x_tmp, y_tmp, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
    else if (method == nre::gpu::FBO_DEPTHSTENCIL) {
        glBlitFramebuffer(0, 0, x_tmp, y_tmp, 0, 0, x_tmp, y_tmp, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    }
    else {
        glBlitFramebuffer(0, 0, x_tmp, y_tmp, 0, 0, x_tmp, y_tmp,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    }
    // if (glGetError() > 0)
    //     cout << glGetError() << endl;
}

void nre::gl3::api_t::clear_framebuffer(std::size_t id, nre::gpu::FRAMEBUFFER_COPY clear, float alpha_value) {
    this->check_fbo_id_(id, "clear_framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, this->fbos_[id].handle);
    glDepthMask(GL_TRUE);
    glColorMask(1, 1, 1, 1);
    glClearColor(0.0f, 0.0f, 0.0f, alpha_value);

    if (clear == nre::gpu::FBO_ALL) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else if (clear == nre::gpu::FBO_COLOR) {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    else if (clear == nre::gpu::FBO_DEPTHSTENCIL) {
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else if (clear == nre::gpu::FBO_COLORSTENCIL) {

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else {
    }
}

void nre::gl3::api_t::discard_framebuffer(std::size_t id) {
    // TODO
    /*if (id != 0) {
        this->check_fbo_id_(id, "discard_framebuffer");
        glBindFramebuffer(GL_FRAMEBUFFER, this->fbos_[id].handle);
        glDiscardFramebufferEXT(GL_FRAMEBUFFER, this->fbos_[id].handle);
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDiscardFramebufferEXT(GL_FRAMEBUFFER, this->fbos_[id].handle);
    }*/
}

void nre::gl3::api_t::use_framebuffer(std::size_t id) {
    if (id != 0) {
        this->check_fbo_id_(id, "use_framebuffer");
        glBindFramebuffer(GL_FRAMEBUFFER, this->fbos_[id].handle);
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void nre::gl3::api_t::delete_framebuffer(std::size_t id) {
    this->check_fbo_id_(id, "delete_framebuffer");
    glDeleteFramebuffers(1, &this->fbos_[id].handle);
    this->fbos_.erase(id);
}

//---------------------Shader Programs-----------------------------//

void nre::gl3::api_t::set_program_vs(std::size_t id, nre::gpu::vertex_shader_t vs) {
    this->check_prog_id_(id, "set_program_vs");

    this->progs_[id].vs_setup = false;
    this->progs_[id].setup    = false;
    this->progs_[id].vs       = vs;
}
void nre::gl3::api_t::set_program_fs(std::size_t id, nre::gpu::pixel_shader_t fs) {
    this->check_prog_id_(id, "set_program_fs");

    this->progs_[id].fs_setup = false;
    this->progs_[id].setup    = false;
    this->progs_[id].fs       = fs;
}

void nre::gl3::api_t::set_program_vs(std::size_t id, std::string data) {

    data = data + "";
    this->check_prog_id_(id, "set_program_vs - internal");
    GLuint shader_id;
    if (!this->progs_[id].vs_setup)
        shader_id = glCreateShader(GL_VERTEX_SHADER);
    else
        shader_id = this->progs_[id].vs_handle;

    this->progs_[id].vs_setup  = true;
    this->progs_[id].vs_handle = shader_id;

    /// compile and attach shader
    const char* c_str = data.c_str();

    GLint data_size = data.length();
    glShaderSource(shader_id, 1, &c_str, &data_size);
    glCompileShader(shader_id);

    /// IMPORTANT: get shader compiler info
    int parameters = -1;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &parameters);
    if (parameters != GL_TRUE) {

        int  max_length    = 2048;
        int  actual_length = 0;
        char log[2048];
        glGetShaderInfoLog(shader_id, max_length, &actual_length, log);

        stringstream ss;
        ss << "[NRE GL Shader Warning] in vertex shader: " << this->progs_[id].vs.info() << endl << log << endl;
        cout << ss.str();
        OE_WriteToLog(ss.str());
    }
}

// void set_programGS(std::size_t, FE_GPU_Shader){}
void nre::gl3::api_t::set_program_fs(std::size_t id, std::string data) {

    data = data + "";
    this->check_prog_id_(id, "set_program_fs - internal");
    GLuint shader_id;
    if (!this->progs_[id].fs_setup)
        shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    else
        shader_id = this->progs_[id].fs_handle;

    this->progs_[id].fs_setup  = true;
    this->progs_[id].fs_handle = shader_id;

    /// compile and attach shader
    const char* c_str     = data.c_str();
    GLint       data_size = data.length();
    glShaderSource(shader_id, 1, &c_str, &data_size);
    glCompileShader(shader_id);

    /// IMPORTANT: get shader compiler info
    int parameters = -1;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &parameters);
    if (parameters != GL_TRUE) {

        int  max_length    = 2048;
        int  actual_length = 0;
        char log[2048];
        glGetShaderInfoLog(shader_id, max_length, &actual_length, log);

        stringstream ss;
        ss << "[NRE GL Shader Warning] in pixel shader: " << this->progs_[id].fs.info() << endl << log << endl;
        cout << ss.str();
        OE_WriteToLog(ss.str());
    }
}

// void set_programTCS(std::size_t, FE_GPU_Shader){}
// void set_programTES(std::size_t, FE_GPU_Shader){}

void nre::gl3::api_t::setup_program(std::size_t id) {

    this->check_prog_id_(id, "setup_program");

    // ignore if the program is already setup
    if (this->progs_[id].setup) return;

    // setup vertex shader
    if (!this->progs_[id].vs_setup) {
        if (this->vs_db_.count(this->progs_[id].vs) == 0) {

            // vertex shader does not exist, make a new entry
            this->set_program_vs(id, this->progs_[id].vs.gen_shader());
            this->vs_db_[this->progs_[id].vs] = this->progs_[id].vs_handle;
        }
        else {

            // vertex shader already exists, reuse that
            this->progs_[id].vs_handle = this->vs_db_[this->progs_[id].vs];
            this->progs_[id].vs_setup  = true;
        }
    }

    // setup pixel (fragment) shader
    if (!this->progs_[id].fs_setup) {
        if (this->fs_db_.count(this->progs_[id].fs) == 0) {

            // pixel (fragment) shader does not exist, make a new entry
            this->set_program_fs(id, this->progs_[id].fs.gen_shader());

            this->fs_db_[this->progs_[id].fs] = this->progs_[id].fs_handle;
            this->progs_[id].fs_setup         = true;
        }
        else {

            // pixel (fragment) shader already exists, reuse that
            this->progs_[id].fs_handle = this->fs_db_[this->progs_[id].fs];
            this->progs_[id].fs_setup  = true;
        }
    }

    // check if program already exists
    if (this->prog_db_.count(this->progs_[id]) > 0) {

        this->progs_[id].handle = this->prog_db_[this->progs_[id]].handle;
        this->progs_[id].setup  = true;
        return;
    }
    else {
        this->progs_[id].handle = glCreateProgram();
        // this->prog_db_[this->progs_[id]] = this->progs_[id].handle;
        this->prog_db_[this->progs_[id]]        = nre::gl3::program_data_t();
        this->prog_db_[this->progs_[id]].handle = this->progs_[id].handle;
    }

    // make sure that the vertex and fragment shaders actually compile
    // in case the program does not already exist
    assert(this->progs_[id].vs_setup && this->progs_[id].fs_setup);

    glAttachShader(this->progs_[id].handle, this->progs_[id].vs_handle);

    // Technically a fragment/pixel shader is optional, but it is a must in OpenGL ES
    // This should be the case sometimes (for example in the Z_PREPASS program)
    bool isES          = nre::gpu::get_api() == nre::gpu::GLES;
    bool isUndefinedFS = this->progs_[id].fs.type == nre::gpu::FS_UNDEFINED;

    if ((isES) || (!isES && !isUndefinedFS)) glAttachShader(this->progs_[id].handle, this->progs_[id].fs_handle);

    glLinkProgram(this->progs_[id].handle);


    /// IMPORTANT: check if program is linked correctly
    int parameters = -1;
    glGetProgramiv(this->progs_[id].handle, GL_LINK_STATUS, &parameters);
    if (GL_TRUE != parameters) {

        int  max_length    = 2048;
        int  actual_length = 0;
        char log[2048];
        glGetProgramInfoLog(this->progs_[id].handle, max_length, &actual_length, log);
        cout << log << endl;
        OE_WriteToLog(log);
    }

    /// IMPORTANT: make sure program is runnable
    glValidateProgram(this->progs_[id].handle);
    parameters = -1;
    glGetProgramiv(this->progs_[id].handle, GL_VALIDATE_STATUS, &parameters);
    if (GL_TRUE != parameters) {

        int  max_length    = 2048;
        int  actual_length = 0;
        char log[2048];
        glGetProgramInfoLog(this->progs_[id].handle, max_length, &actual_length, log);

        stringstream ss;
        ss << "[NRE GL Shader Linking Warning] with shaders: " << this->progs_[id].vs.info() << " "
           << this->progs_[id].fs.info() << endl
           << log << endl;
        cout << ss.str();
        OE_WriteToLog(ss.str());
    }
    glUseProgram(this->progs_[id].handle);
    this->active_prog_ = this->progs_[id].handle;

    /// get all active uniform blocks
    this->get_program_all_uniforms_(id);

    this->progs_[id].setup = true;
}

void nre::gl3::api_t::delete_program(std::size_t id) {

    this->check_prog_id_(id, "delete_program");

    this->active_prog_ = 0;
    this->progs_.erase(id);
}

//---------------------Draw calls-----------------------------//

void nre::gl3::api_t::draw(std::size_t prog_id, std::size_t vao_id, int offset, int count) {

    this->check_prog_id_(prog_id, "draw (non-indexed)");
    this->check_vao_id_(vao_id, "draw (non-indexed)");
    this->check_draw_range_(vao_id, this->get_vao_size(vao_id), offset, count, "draw (non-indexed)");
    this->setup_program(prog_id);

    if (this->active_prog_ != this->progs_[prog_id].handle) {
        glUseProgram(this->progs_[prog_id].handle);
        this->active_prog_ = this->progs_[prog_id].handle;
    }
    if (this->active_vao_ != this->vaos_[vao_id].handle) {
        glBindVertexArray(this->vaos_[vao_id].handle);
        this->active_vao_ = this->vaos_[vao_id].handle;
        this->active_vbo_ = 0;
    }
    glDrawArrays(GL_TRIANGLES, offset, count);
}

void nre::gl3::api_t::draw(std::size_t prog_id, std::size_t vao_id) {

    this->check_prog_id_(prog_id, "draw (non-indexed)");
    this->check_vao_id_(vao_id, "draw (non-indexed)");
    this->setup_program(prog_id);

    if (this->active_prog_ != this->progs_[prog_id].handle) {
        glUseProgram(this->progs_[prog_id].handle);
        this->active_prog_ = this->progs_[prog_id].handle;
    }
    if (this->active_vao_ != this->vaos_[vao_id].handle) {
        glBindVertexArray(this->vaos_[vao_id].handle);
        this->active_vao_ = this->vaos_[vao_id].handle;
        this->active_vbo_ = 0;
    }
    glDrawArrays(GL_TRIANGLES, 0, this->get_vao_size(vao_id));
}

void nre::gl3::api_t::draw(std::size_t prog_id, std::size_t vao_id, std::size_t ibo_id, int offset, int count) {

    this->check_prog_id_(prog_id, "draw (indexed)");
    this->check_vao_id_(vao_id, "draw (indexed)");
    this->check_ibo_id_(ibo_id, "draw (indexed)");
    this->check_draw_range_(vao_id, this->ibos_[ibo_id].size, offset, count, "draw (indexed)");
    this->setup_program(prog_id);

    if (this->active_prog_ != this->progs_[prog_id].handle) {
        glUseProgram(this->progs_[prog_id].handle);
        this->active_prog_ = this->progs_[prog_id].handle;
    }
    if (this->active_vao_ != this->vaos_[vao_id].handle) {
        glBindVertexArray(this->vaos_[vao_id].handle);
        this->active_vao_ = this->vaos_[vao_id].handle;
        this->active_vbo_ = 0;
    }
    if (this->vao_ibos_[this->active_vao_] != this->ibos_[ibo_id].handle) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos_[ibo_id].handle);
        this->vao_ibos_[this->active_vao_] = this->ibos_[ibo_id].handle;
    }
    glDrawRangeElements(GL_TRIANGLES, offset, count, 1, GL_UNSIGNED_INT, (GLvoid*)NULL);
}

void nre::gl3::api_t::draw(std::size_t prog_id, std::size_t vao_id, std::size_t ibo_id) {

    this->check_prog_id_(prog_id, "draw (indexed)");
    this->check_vao_id_(vao_id, "draw (indexed)");
    this->check_ibo_id_(ibo_id, "draw (indexed)");
    this->setup_program(prog_id);

    if (this->active_prog_ != this->progs_[prog_id].handle) {
        glUseProgram(this->progs_[prog_id].handle);
        this->active_prog_ = this->progs_[prog_id].handle;
    }
    if (this->active_vao_ != this->vaos_[vao_id].handle) {
        glBindVertexArray(this->vaos_[vao_id].handle);
        this->active_vao_ = this->vaos_[vao_id].handle;
        this->active_vbo_ = 0;
    }
    if (this->vao_ibos_[this->active_vao_] != this->ibos_[ibo_id].handle) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos_[ibo_id].handle);
        this->vao_ibos_[this->active_vao_] = this->ibos_[ibo_id].handle;
    }

    glDrawElements(GL_TRIANGLES, this->ibos_[ibo_id].size, GL_UNSIGNED_INT, (GLvoid*)NULL);
}

void nre::gl3::api_t::draw_instanced(std::size_t prog_id, std::size_t vao_id, std::size_t instancecount) {

    this->check_prog_id_(prog_id, "draw (instanced-indexed)");
    this->check_vao_id_(vao_id, "draw (instanced-indexed)");
    this->setup_program(prog_id);

    if (this->active_prog_ != this->progs_[prog_id].handle) {
        glUseProgram(this->progs_[prog_id].handle);
        this->active_prog_ = this->progs_[prog_id].handle;
    }
    if (this->active_vao_ != this->vaos_[vao_id].handle) {
        glBindVertexArray(this->vaos_[vao_id].handle);
        this->active_vao_ = this->vaos_[vao_id].handle;
        this->active_vbo_ = 0;
    }
    glDrawArraysInstanced(GL_TRIANGLES, 0, this->get_vao_size(vao_id), instancecount);
}

void nre::gl3::api_t::draw_instanced(std::size_t prog_id, std::size_t vao_id, std::size_t ibo_id, std::size_t instancecount) {

    this->check_prog_id_(prog_id, "draw (instanced-indexed)");
    this->check_vao_id_(vao_id, "draw (instanced-indexed)");
    this->check_ibo_id_(ibo_id, "draw (instanced-indexed)");
    this->setup_program(prog_id);

    if (this->active_prog_ != this->progs_[prog_id].handle) {
        glUseProgram(this->progs_[prog_id].handle);
        this->active_prog_ = this->progs_[prog_id].handle;
    }
    if (this->active_vao_ != this->vaos_[vao_id].handle) {
        glBindVertexArray(this->vaos_[vao_id].handle);
        this->active_vao_ = this->vaos_[vao_id].handle;
        this->active_vbo_ = 0;
    }
    if (this->vao_ibos_[this->active_vao_] != this->ibos_[ibo_id].handle) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibos_[ibo_id].handle);
        this->vao_ibos_[this->active_vao_] = this->ibos_[ibo_id].handle;
    }

    glDrawElementsInstanced(GL_TRIANGLES, this->ibos_[ibo_id].size, GL_UNSIGNED_INT, (GLvoid*)NULL, instancecount);
}

void nre::gl3::api_t::set_render_mode(nre::gpu::RENDERMODE rendermode) {

    if (rendermode == nre::gpu::Z_PREPASS_BACKFACE) {
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); /// cull back face
        glFrontFace(GL_CCW);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glColorMask(0, 0, 0, 0);
        glDepthMask(GL_TRUE);
    }
    else if (rendermode == nre::gpu::REGULAR_FRONTFACE) {
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);

        glEnable(GL_DEPTH_TEST); // enable depth-testing
        glDepthFunc(GL_LESS);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT); /// cull back face
        glFrontFace(GL_CCW);
    }
    else if (rendermode == nre::gpu::REGULAR_BACKFACE) {
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);

        glEnable(GL_DEPTH_TEST); // enable depth-testing
        glDepthFunc(GL_LESS);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); /// cull back face
        glFrontFace(GL_CCW);
    }
    else if (rendermode == nre::gpu::AFTERPREPASS_BACKFACE) {
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_FALSE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); /// cull back face
        glFrontFace(GL_CCW);
    }
    else if (rendermode == nre::gpu::REGULAR_BOTH) {
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);

        glEnable(GL_DEPTH_TEST); // enable depth-testing
        glDepthFunc(GL_LESS);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_TRUE);

        glDisable(GL_CULL_FACE);
    }
    else if (rendermode == nre::gpu::FULLSCREEN_QUAD) {
        glDisable(GL_BLEND);

        glDisable(GL_STENCIL_TEST);
        glDisable(GL_DEPTH_TEST);
        // glDepthFunc(GL_LEQUAL);
        glColorMask(1, 1, 1, 1);
        glDepthMask(GL_TRUE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); /// cull back face
        glFrontFace(GL_CCW);
    }
    else if (rendermode == nre::gpu::LIGHT_PREPASS) {
        glDisable(GL_BLEND);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_REPLACE, GL_ZERO);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glColorMask(0, 0, 0, 0);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); /// cull back face
        glFrontFace(GL_CCW);
    }
    else if (rendermode == nre::gpu::LIGHT_AFTERPASS) {
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

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT); /// cull front face
        glFrontFace(GL_CCW);
    }
    else if (rendermode == nre::gpu::LIGHT_PREPASS_2) {
        glDisable(GL_BLEND);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 2, 0xFF);
        // glStencilFunc(GL_GEQUAL, 2, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_REPLACE, GL_ZERO);
        // glStencilOp(GL_KEEP, GL_INCR, GL_DECR);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glColorMask(0, 0, 0, 0);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); /// cull back face
        glFrontFace(GL_CCW);
    }
    else if (rendermode == nre::gpu::LIGHT_AFTERPASS_RG) {
        glEnable(GL_BLEND);
        glBlendEquation(GL_MAX);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_GREATER, 2, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_GEQUAL);
        glColorMask(1, 1, 0, 0);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT); /// cull front face
        glFrontFace(GL_CCW);
    }
    else if (rendermode == nre::gpu::LIGHT_AFTERPASS_BA) {
        glEnable(GL_BLEND);
        glBlendEquation(GL_MAX);

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 0, 0xFF);
        glStencilMask(0xFF);
        glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_GEQUAL);
        glColorMask(0, 0, 1, 1);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT); /// cull front face
        glFrontFace(GL_CCW);
    }
    else {
        // TODO
    }
}

void nre::gl3::api_t::use_wireframe(bool value_in) {
#ifndef OE_PLATFORM_WEB
    if (nre::gpu::get_api() != nre::gpu::GLES) {
        if (value_in) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
#endif
}

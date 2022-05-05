#include <OE/Renderer/GL3/api_gl3.h>
#include <OE/Renderer/GLES2/api_gles2.h>
#include <OE/Renderer/api_gpu.h>


void*                 nre::gpu::api          = nullptr;
nre::gpu::info_struct nre::gpu::backend_info = nre::gpu::info_struct();

int nre::gpu::get_minor_api_version() {
    return nre::gpu::backend_info.minor;
}

int nre::gpu::get_major_api_version() {
    return nre::gpu::backend_info.major;
}

nre::gpu::vertex_layout_input::vertex_layout_input() {
}

nre::gpu::vertex_layout_input::vertex_layout_input(std::size_t vbo, std::size_t offseta, unsigned int amounta,
                                                   unsigned int stridea) {
    this->vertex_buffer = vbo;
    this->offset        = offseta;
    this->amount        = amounta;
    this->stride        = stridea;
}

nre::gpu::info_struct nre::gpu::get_backend_info() {
    return nre::gpu::backend_info;
}

std::string nre::gpu::get_underlying_api_name() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->get_rendering_api();
    case GLES2:
        return static_cast<nre::gles2::api_t*>(nre::gpu::api)->get_rendering_api();
    default:
        return "undefined";
    }
}

nre::gpu::SHADER_BACKEND nre::gpu::get_api() {
    return nre::gpu::backend_info.underlying_api;
}


std::size_t nre::gpu::new_vertex_buf() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->new_vertex_buffer();
    case GLES2:
        return static_cast<nre::gles2::api_t*>(nre::gpu::api)->new_vertex_buffer();
    default:
        return 0;
    }
}

std::size_t nre::gpu::new_program() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->new_program();
    case GLES2:
        return static_cast<nre::gles2::api_t*>(nre::gpu::api)->new_program();

    default:
        return 0;
    }
}

std::size_t nre::gpu::new_vertex_layout() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->new_vertex_layout();
    case GLES2:
        return static_cast<nre::gles2::api_t*>(nre::gpu::api)->new_vertex_layout();
    default:
        return 0;
    }
}

std::size_t nre::gpu::new_index_buf() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->new_index_buffer();
    case GLES2:
        return static_cast<nre::gles2::api_t*>(nre::gpu::api)->new_index_buffer();

    default:
        return 0;
    }
}

std::size_t nre::gpu::new_uniform_buf() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->new_uniform_buffer();
    case GLES2:
        throw nre::gpu::unimplemented_function("new_uniform_buf", "OpenGL ES 2", "Uniform buffers are not supported on ES 2.");
    default:
        return 0;
    }
}

std::size_t nre::gpu::new_framebuffer() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->new_framebuffer();
    case GLES2:
        return static_cast<nre::gles2::api_t*>(nre::gpu::api)->new_framebuffer();

    default:
        return 0;
    }
}

std::size_t nre::gpu::new_texture() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->new_texture();
    case GLES2:
        return static_cast<nre::gles2::api_t*>(nre::gpu::api)->new_texture();
    default:
        return 0;
    }
}

std::size_t nre::gpu::new_renderbuffer() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->new_renderbuffer();
    case GLES2:
        return static_cast<nre::gles2::api_t*>(nre::gpu::api)->new_renderbuffer();

    default:
        return 0;
    }
}

// init update destroy
bool nre::gpu::init(SHADER_BACKEND backend_in, int major, int minor) {

    if (nre::gpu::api != nullptr) {
        return false;
    }

    nre::gpu::info_struct info_backend;
    info_backend.underlying_api = backend_in;
    info_backend.major          = major;
    info_backend.minor          = minor;
    nre::gpu::backend_info      = info_backend;

    // nre::gpu::shader_base_t::init(backend_in, major, minor);

    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        nre::gpu::api = static_cast<void*>(new nre::gl3::api_t(nre::gpu::backend_info));
        break;
    case GLES2:
        nre::gpu::api = static_cast<void*>(new nre::gles2::api_t(nre::gpu::backend_info));
        break;
    default:
        return false;
    }

    nre::gpu::backend_info.has_init = true;

    return true;
}

void nre::gpu::update(uint32_t x, uint32_t y, bool sanity_checks) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->update(x, y, sanity_checks);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->update(x, y, sanity_checks);
        break;
    default:
        return;
    }
}

void nre::gpu::destroy() {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->destroy();
        delete static_cast<nre::gl3::api_t*>(nre::gpu::api);
        nre::gpu::api = nullptr;
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->destroy();
        delete static_cast<nre::gles2::api_t*>(nre::gpu::api);
        nre::gpu::api = nullptr;
        break;
    default:
        return;
    }
}

// renderbuffers
void nre::gpu::set_renderbuffer_textype(std::size_t id, nre::gpu::TEXTURE_TYPE a_type, int x, int y) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_renderbuffer_textype(id, a_type, x, y);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_renderbuffer_textype(id, a_type, x, y);
        break;
    default:
        return;
    }
}

void nre::gpu::set_framebuffer_renderbuffer(std::size_t fbo_id, std::size_t rbo_id, int slot) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_framebuffer_renderbuffer(fbo_id, rbo_id, slot);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_framebuffer_renderbuffer(fbo_id, rbo_id, slot);
        break;
    default:
        return;
    }
}

// vertex buffers
void nre::gpu::set_vertex_buf_memory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_vertex_buffer_memory(id, memory_size, buf_usage);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_vertex_buffer_memory(id, memory_size, buf_usage);
        break;
    default:
        return;
    }
}

void nre::gpu::set_vertex_buf_data(std::size_t id, const std::vector<float>& data, std::size_t offset) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_vertex_buffer_data(id, data, offset);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_vertex_buffer_data(id, data, offset);
        break;
    default:
        return;
    }
}

void nre::gpu::set_vertex_buf_memory_and_data(std::size_t id, const std::vector<float>& data, BUFFER_USAGE buf_usage) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_vertex_buffer_memory_data(id, data, buf_usage);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_vertex_buffer_memory_data(id, data, buf_usage);
        break;
    default:
        return;
    }
}

void nre::gpu::del_vertex_buf(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->delete_vertex_buffer(id);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->delete_vertex_buffer(id);
        break;
    default:
        return;
    }
}

// index buffers
void nre::gpu::set_index_buf_memory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_index_buffer_memory(id, memory_size, buf_usage);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_index_buffer_memory(id, memory_size, buf_usage);
        break;
    default:
        return;
    }
}

void nre::gpu::set_index_buf_data(std::size_t id, const std::vector<uint32_t>& data, std::size_t offset) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_index_buffer_data(id, data, offset);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_index_buffer_data(id, data, offset);
        break;
    default:
        return;
    }
}

void nre::gpu::set_index_buf_memory_and_data(std::size_t id, const std::vector<uint32_t>& data, BUFFER_USAGE buf_usage) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_index_buffer_memory_data(id, data, buf_usage);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_index_buffer_memory_data(id, data, buf_usage);
        break;
    default:
        return;
    }
}

void nre::gpu::del_index_buf(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->delete_index_buffer(id);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->delete_index_buffer(id);
        break;
    default:
        return;
    }
}

// uniform buffers
void nre::gpu::set_uniform_buf_memory(std::size_t id, std::size_t memory_size, nre::gpu::BUFFER_USAGE buf_usage) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_uniform_buffer_memory(id, memory_size, buf_usage);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("set_uniform_buf_memory", "OpenGL ES 2",
                                               "Uniform buffers are not supported on ES 2.");
    default:
        return;
    }
}

void nre::gpu::set_uniform_buf_data(std::size_t id, const std::vector<float>& data, std::size_t offset) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_uniform_buffer_data(id, data, offset);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("set_uniform_buf_data", "OpenGL ES 2",
                                               "Uniform buffers are not supported on ES 2.");
    default:
        return;
    }
}

void nre::gpu::set_uniform_buf_data(std::size_t id, const std::vector<uint32_t>& data, std::size_t offset) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_uniform_buffer_data(id, data, offset);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("set_uniform_buf_data", "OpenGL ES 2",
                                               "Uniform buffers are not supported on ES 2.");
    default:
        return;
    }
}

void nre::gpu::set_uniform_buf_memory_and_data(std::size_t id, const std::vector<uint32_t>& data, BUFFER_USAGE buf_usage) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_uniform_buffer_memory(id, data.size(), buf_usage);
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_uniform_buffer_data(id, data, 0);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("set_uniform_buf_memory_and_data", "OpenGL ES 2",
                                               "Uniform buffers are not supported on ES 2.");
    default:
        return;
    }
}

void nre::gpu::set_uniform_buf_memory_and_data(std::size_t id, const std::vector<float>& data, BUFFER_USAGE buf_usage) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_uniform_buffer_memory(id, data.size(), buf_usage);
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_uniform_buffer_data(id, data, 0);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("set_uniform_buf_memory_and_data", "OpenGL ES 2",
                                               "Uniform buffers are not supported on ES 2.");
    default:
        return;
    }
}

void nre::gpu::del_uniform_buf(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->delete_uniform_buffer(id);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("del_uniform_buf", "OpenGL ES 2", "Uniform buffers are not supported on ES 2.");
    default:
        return;
    }
}

void nre::gpu::set_uniform_buf_state(std::size_t id, std::size_t program, int slot, std::size_t offset, std::size_t length) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_uniform_block_state(id, program, slot, offset, length);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("set_uniform_buf_state", "OpenGL ES 2",
                                               "Uniform buffers are not supported on ES 2.");
    default:
        return;
    }
}
// program uniform buffer/texture state
void nre::gpu::set_program_uniform_buf_slot(std::size_t id, std::string_view ubo, int slot) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_program_uniform_block_slot(id, ubo, slot);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("set_program_uniform_buf_slot", "OpenGL ES 2",
                                               "Uniform buffers are not supported on ES 2.");
    default:
        return;
    }
}
int nre::gpu::get_program_uniform_buf_slot(std::size_t id, std::string_view name) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->get_program_uniform_block_slot(id, name);
    case GLES2:
        throw nre::gpu::unimplemented_function("get_program_uniform_buf_slot", "OpenGL ES 2",
                                               "Uniform buffers are not supported on ES 2.");
    default:
        return -2;
    }
}
void nre::gpu::set_program_texture_slot(std::size_t id, std::string_view name, int slot) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_program_texture_slot(id, name, slot);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_program_texture_slot(id, name, slot);
        break;
    default:
        return;
    }
}

// program uniform data
void nre::gpu::set_program_uniform_data(std::size_t id, std::string_view name, uint32_t data) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_program_uniform_data(id, name, data);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_program_uniform_data(id, name, data);
        break;
    default:
        return;
    }
}
void nre::gpu::set_program_uniform_data(std::size_t id, std::string_view name, float data) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_program_uniform_data(id, name, data);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_program_uniform_data(id, name, data);
        break;
    default:
        return;
    }
}
void nre::gpu::set_program_uniform_data(std::size_t id, std::string_view name, const std::vector<uint32_t>& data) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_program_uniform_data(id, name, data);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("set_program_uniform_data", "OpenGL ES 2",
                                               "Uniform integer array data is not implemented due to boredom.");
    default:
        return;
    }
}
void nre::gpu::set_program_uniform_data(std::size_t id, std::string_view name, const std::vector<float>& data) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        throw nre::gpu::unimplemented_function("set_program_uniform_data",
                                               static_cast<nre::gl3::api_t*>(nre::gpu::api)->get_rendering_api(),
                                               "Uniform float array data is not implemented due to boredom.");
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_program_uniform_data(id, name, data);
        break;
    default:
        return;
    }
}

int nre::gpu::get_program_uniform_slot(std::size_t id, std::string_view name) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        return static_cast<nre::gl3::api_t*>(nre::gpu::api)->get_program_uniform_slot(id, name);
    case GLES2:
        return static_cast<nre::gles2::api_t*>(nre::gpu::api)->get_program_uniform_slot(id, name);

    default:
        return -2;
    }
}

// vertex layout
void nre::gpu::set_vertex_layout_format(std::size_t id, std::vector<vertex_layout_input> vao_input) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_vertex_layout_format(id, vao_input);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_vertex_layout_format(id, vao_input);
        break;
    default:
        return;
    }
}
void nre::gpu::del_vertex_layout(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->delete_vertex_layout(id);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->delete_vertex_layout(id);
        break;
    default:
        return;
    }
}

// texture format
void nre::gpu::set_texture_format(std::size_t id, TEXTURE_TYPE textype, TEXTURE_FILTER texfil, uint32_t x, uint32_t y,
                                  int mipmaps) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_texture_format(id, textype, texfil, x, y, mipmaps);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_texture_format(id, textype, texfil, x, y, mipmaps);
        break;
    default:
        return;
    }
}
void nre::gpu::set_texture_slot(std::size_t id, int slot) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_texture_slot(id, slot);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_texture_slot(id, slot);
        break;
    default:
        return;
    }
}
void nre::gpu::del_texture(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->delete_texture(id);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->delete_texture(id);
        break;
    default:
        return;
    }
}

// framebuffer texture
void nre::gpu::set_framebuffer_texture(std::size_t id, std::size_t tex, int slot) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_framebuffer_texture(id, tex, slot);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_framebuffer_texture(id, tex, slot);
        break;
    default:
        return;
    }
}
void nre::gpu::copy_framebuffer(std::size_t src, std::size_t target, FRAMEBUFFER_COPY method) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->copy_framebuffer(src, target, method);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("copy_framebuffer", "OpenGL ES 2",
                                               "Implementing this manually would give suboptimal performance on ES 2.");
    default:
        return;
    }
}
void nre::gpu::use_framebuffer(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->use_framebuffer(id);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->use_framebuffer(id);
        break;
    default:
        return;
    }
}

void nre::gpu::discard_framebuffer(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->discard_framebuffer(id);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->discard_framebuffer(id);
        break;
    default:
        return;
    }
}

void nre::gpu::clear_framebuffer(std::size_t id, nre::gpu::FRAMEBUFFER_COPY fbcopy, float alpha) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->clear_framebuffer(id, fbcopy, alpha);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->clear_framebuffer(id, fbcopy, alpha);
        break;
    default:
        return;
    }
}
void nre::gpu::del_framebuffer(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->delete_framebuffer(id);
        break;
    case GLES2:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->delete_framebuffer(id);
        break;
    default:
        return;
    }
}

// vertex shaders
void nre::gpu::set_program_vertex_shader(std::size_t id, nre::gpu::vertex_shader_t vs) {
    // std::cout << "VS HASH " << std::bitset<64>(vs.gen_hash()) << " " << vs.info() << std::endl;
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_program_vs(id, vs);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_program_vs(id, vs);
        break;
    default:
        return;
    }
}
void nre::gpu::set_program_pixel_shader(std::size_t id, nre::gpu::pixel_shader_t fs) {
    // std::cout << "FS HASH " << std::bitset<64>(fs.gen_hash()) << " " << fs.info() << std::endl;
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_program_fs(id, fs);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_program_fs(id, fs);
        break;
    default:
        return;
    }
}
void nre::gpu::setup_program(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->setup_program(id);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->setup_program(id);
        break;
    default:
        return;
    }
}
void nre::gpu::del_program(std::size_t id) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->delete_program(id);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->delete_program(id);
        break;
    default:
        return;
    }
}

void nre::gpu::draw(nre::gpu::draw_call dcall) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        if ((dcall.offset == 0) and (dcall.amount == 0)) {
            if (dcall.index_buf == 0)
                static_cast<nre::gl3::api_t*>(nre::gpu::api)->draw(dcall.program, dcall.vertex_layout);
            else
                static_cast<nre::gl3::api_t*>(nre::gpu::api)->draw(dcall.program, dcall.vertex_layout, dcall.index_buf);
        }
        else {
            if (dcall.index_buf == 0)
                static_cast<nre::gl3::api_t*>(nre::gpu::api)
                    ->draw(dcall.program, dcall.vertex_layout, dcall.offset, dcall.amount);
            else
                static_cast<nre::gl3::api_t*>(nre::gpu::api)
                    ->draw(dcall.program, dcall.vertex_layout, dcall.index_buf, dcall.offset, dcall.amount);
        }
        break;

    default:
        return;
    }
}

void nre::gpu::draw(std::size_t prog, std::size_t vao) {

    nre::gpu::draw_call dcall;
    dcall.program       = prog;
    dcall.vertex_layout = vao;

    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->draw(prog, vao);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->draw(dcall);
        break;

    default:
        return;
    }
}
void nre::gpu::draw(std::size_t prog, std::size_t vao, std::size_t ibo) {

    nre::gpu::draw_call dcall;
    dcall.program       = prog;
    dcall.vertex_layout = vao;
    dcall.index_buf     = ibo;

    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->draw(prog, vao, ibo);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->draw(dcall);
        break;
    default:
        return;
    }
}

void nre::gpu::draw_instanced(draw_call dcall, int count) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        if ((dcall.offset == 0) and (dcall.amount == 0)) {
            if (dcall.index_buf == 0)
                static_cast<nre::gl3::api_t*>(nre::gpu::api)->draw_instanced(dcall.program, dcall.vertex_layout, count);
            else
                static_cast<nre::gl3::api_t*>(nre::gpu::api)
                    ->draw_instanced(dcall.program, dcall.vertex_layout, count, dcall.index_buf);
        }
        else {
            if (dcall.index_buf == 0)
                // TODO: Error unimplemented function
                throw nre::gpu::unimplemented_function("draw_instanced",
                                                       static_cast<nre::gl3::api_t*>(nre::gpu::api)->get_rendering_api(),
                                                       "No instanced range rendering supported.");
            else
                // TODO: Error unimplemented function
                throw nre::gpu::unimplemented_function("draw_instanced",
                                                       static_cast<nre::gl3::api_t*>(nre::gpu::api)->get_rendering_api(),
                                                       "No instanced range rendering supported.");
        }
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("draw_instanced", "OpenGL ES 2", "Instanced rendering is not supported.");
    default:
        return;
    }
}

void nre::gpu::draw_instanced(std::size_t prog, std::size_t vao, std::size_t ibo, int count) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->draw_instanced(prog, vao, ibo, count);
        break;
    case GLES2:
        throw nre::gpu::unimplemented_function("draw_instanced", "OpenGL ES 2", "Instanced rendering is not supported.");
    default:
        return;
    }
}

void nre::gpu::set_render_mode(RENDERMODE mode) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->set_render_mode(mode);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->set_render_mode(mode);
        break;
    default:
        return;
    }
}

void nre::gpu::use_wireframe(bool mode) {
    switch (nre::gpu::get_api()) {
    case GL:
    case GLES:
        static_cast<nre::gl3::api_t*>(nre::gpu::api)->use_wireframe(mode);
        break;
    case GLES2:
        static_cast<nre::gles2::api_t*>(nre::gpu::api)->use_wireframe(mode);
        break;
    default:
        return;
    }
}

#ifndef api_t_H_INCLUDED
#define api_t_H_INCLUDED


#include <OE/Renderer/GL3/shaders_gl3.h>
#include <OE/Renderer/api_gpu.h>
#include <OE/types/libs_oe.h>
#include <utility>

namespace nre { namespace gl3 {
    struct renderbuffer_t {
        GLuint                 handle{0};
        nre::gpu::TEXTURE_TYPE type;
        int                    x{0};
        int                    y{0};

        bool has_not_changed(nre::gpu::TEXTURE_TYPE, int, int);
    };

    struct vertex_buffer_t {
        GLuint                 handle;
        std::size_t            size;
        nre::gpu::BUFFER_USAGE usage;
    };

    struct index_buffer_t {
        GLuint                 handle;
        std::size_t            size;
        nre::gpu::BUFFER_USAGE usage;
    };

    struct framebuffer_t {
        GLuint handle;
        // int components{0};
        // bool depth{false};
        // bool stencil{false};
        std::size_t texture{0};
    };

    struct texture_t {
        GLuint                   handle;
        nre::gpu::TEXTURE_TYPE   type;
        nre::gpu::TEXTURE_FILTER filter;
        int                      x{0};
        int                      y{0};
        int                      mipmaps{0};

        bool has_not_changed(nre::gpu::TEXTURE_TYPE, nre::gpu::TEXTURE_FILTER, int, int, int);
    };

    struct vertex_layout_t {
        GLuint                                     handle;
        std::vector<nre::gpu::vertex_layout_input> layout;
    };

    struct uniform_buffer_t {
        GLuint                 handle;
        std::size_t            size;
        nre::gpu::BUFFER_USAGE usage;
        GLint                  slot;
    };

    struct program_uniform_state_t {
        GLint  slot{0};
        GLenum type{GL_FLOAT}; // unused in Uniform blocks
        size_t size{0};        // unused in Uniform Blocks
    };

    struct program_data_t {
        GLuint handle{0};

        std::unordered_map<std::string, program_uniform_state_t, oe_string_hash, std::equal_to<>> uniform_blocks;
        std::unordered_map<std::string, GLuint, oe_string_hash, std::equal_to<>>                  uniform_block_indices;
        bool has_uniform_block(std::string_view);

        std::unordered_map<std::string, program_uniform_state_t, oe_string_hash, std::equal_to<>> uniforms;
        bool                                                                                      has_uniform(std::string_view);
    };

    struct program_t {

        nre::gpu::vertex_shader_t vs;
        GLuint                    vs_handle{0};
        bool                      vs_setup{false};

        nre::gpu::pixel_shader_t fs;
        GLuint                   fs_handle{0};
        bool                     fs_setup{false};

        bool   setup{false};
        bool   prog_created{false};
        GLuint handle{0};

        // this is needed for it to be in an std::unordered_set
        bool   operator==(const program_t&) const;
        size_t gen_hash() const;
    };
}; }; // namespace nre::gl3

namespace std {
template <>
struct hash<nre::gl3::program_t> {
    auto operator()(const nre::gl3::program_t& xyz) const -> size_t {
        return hash<size_t>{}(xyz.gen_hash());
    }
};
} // namespace std

namespace nre { namespace gl3 {


    GLenum buffer_use(nre::gpu::BUFFER_USAGE);

    class api_t {
    public:
        api_t(nre::gpu::info_struct&);
        ~api_t();

        void update(uint32_t, uint32_t, bool);

        void destroy();

        std::string get_rendering_api();

        std::size_t new_vertex_buffer();
        std::size_t new_vertex_layout();
        std::size_t new_index_buffer();
        std::size_t new_program();
        std::size_t new_uniform_buffer();
        std::size_t new_framebuffer();
        std::size_t new_texture();
        std::size_t new_renderbuffer();

        void set_renderbuffer_textype(std::size_t, nre::gpu::TEXTURE_TYPE, int, int);
        void set_framebuffer_renderbuffer(std::size_t, std::size_t, int);

        void set_vertex_buffer_memory(std::size_t, std::size_t, nre::gpu::BUFFER_USAGE);
        void set_vertex_buffer_data(std::size_t, const std::vector<float>&, std::size_t);
        void set_vertex_buffer_memory_data(std::size_t, const std::vector<float>&, nre::gpu::BUFFER_USAGE);
        void delete_vertex_buffer(std::size_t);

        void set_index_buffer_memory(std::size_t, std::size_t, nre::gpu::BUFFER_USAGE);
        void set_index_buffer_data(std::size_t, const std::vector<uint32_t>&, std::size_t);
        void set_index_buffer_memory_data(std::size_t, const std::vector<uint32_t>&, nre::gpu::BUFFER_USAGE);
        void delete_index_buffer(std::size_t);

        void set_uniform_buffer_memory(std::size_t, std::size_t, nre::gpu::BUFFER_USAGE);
        void set_uniform_buffer_data(std::size_t, const std::vector<float>&, std::size_t);
        void set_uniform_buffer_data(std::size_t, const std::vector<uint32_t>&, std::size_t);

        void set_program_uniform_block_slot(std::size_t, std::string_view, int);
        int  get_program_uniform_block_slot(std::size_t, std::string_view);

        void set_program_texture_slot(std::size_t, std::string_view, int);
        void set_program_uniform_data(std::size_t, std::string_view, uint32_t);
        void set_program_uniform_data(std::size_t, std::string_view, std::vector<uint32_t>);
        int  get_program_uniform_slot(std::size_t, std::string_view);

        void set_uniform_block_state(std::size_t, std::size_t, int, std::size_t, std::size_t);
        void delete_uniform_buffer(std::size_t);

        void set_vertex_layout_format(std::size_t, std::vector<nre::gpu::vertex_layout_input>);
        void delete_vertex_layout(std::size_t);

        void set_texture_format(std::size_t, nre::gpu::TEXTURE_TYPE, nre::gpu::TEXTURE_FILTER, uint32_t, uint32_t, int);
        void set_framebuffer_texture(std::size_t, std::size_t, int);
        void set_texture_slot(std::size_t, int);
        void delete_texture(std::size_t);

        void copy_framebuffer(std::size_t, std::size_t, nre::gpu::FRAMEBUFFER_COPY);
        void use_framebuffer(std::size_t);
        void discard_framebuffer(std::size_t);
        void clear_framebuffer(std::size_t, nre::gpu::FRAMEBUFFER_COPY, float);
        void delete_framebuffer(std::size_t);

        void set_program_vs(std::size_t, nre::gpu::vertex_shader_t);
        void set_program_fs(std::size_t, nre::gpu::pixel_shader_t);

        void set_program_vs(std::size_t, std::string);
        // void set_programGS(std::size_t, FE_GPU_Shader);
        void set_program_fs(std::size_t, std::string);
        // void set_programTCS(std::size_t, FE_GPU_Shader);
        // void set_programTES(std::size_t, FE_GPU_Shader);
        void setup_program(std::size_t);
        void delete_program(std::size_t);

        void draw(std::size_t, std::size_t, int, int);
        void draw(std::size_t, std::size_t);

        void draw(std::size_t, std::size_t, std::size_t, int, int);
        void draw(std::size_t, std::size_t, std::size_t);

        void draw_instanced(std::size_t, std::size_t, std::size_t);
        void draw_instanced(std::size_t, std::size_t, std::size_t, std::size_t);

        void set_render_mode(nre::gpu::RENDERMODE);
        void use_wireframe(bool);

    private:
        std::size_t cur_rbo_{0};
        std::size_t cur_vbo_{0};
        std::size_t cur_ibo_{0};
        std::size_t cur_vao_{0};
        std::size_t cur_ubo_{0};
        std::size_t cur_fbo_{0};
        std::size_t cur_texture_{0};
        std::size_t cur_prog_{0};

        std::unordered_map<std::size_t, renderbuffer_t>   rbos_;
        std::unordered_map<std::size_t, vertex_buffer_t>  vbos_;
        std::unordered_map<std::size_t, index_buffer_t>   ibos_;
        std::unordered_map<std::size_t, vertex_layout_t>  vaos_;
        std::unordered_map<std::size_t, uniform_buffer_t> ubos_;
        std::unordered_map<std::size_t, framebuffer_t>    fbos_;
        std::unordered_map<std::size_t, texture_t>        textures_;
        std::unordered_map<std::size_t, program_t>        progs_;

        std::size_t get_vao_size(std::size_t);

        std::unordered_map<nre::gpu::vertex_shader_t, GLuint> vs_db_;
        std::unordered_map<nre::gpu::pixel_shader_t, GLuint>  fs_db_;
        std::unordered_map<program_t, program_data_t>         prog_db_;

        void check_rbo_id_(std::size_t, std::string_view);
        void check_vbo_id_(std::size_t, std::string_view);
        void check_ubo_id_(std::size_t, std::string_view);
        void check_ibo_id_(std::size_t, std::string_view);

        void check_vbo_offset_length_(std::size_t, std::size_t, std::string_view);
        void check_ubo_offset_length_(std::size_t, std::size_t, std::string_view);
        void check_ibo_offset_length_(std::size_t, std::size_t, std::string_view);

        void check_vao_id_(std::size_t, std::string_view);
        void check_prog_id_(std::size_t, std::string_view);
        void check_prog_complete_(std::size_t, std::string_view);

        void check_prog_uniform_block_(std::size_t, std::string_view, std::string_view);
        void check_prog_uniform_(std::size_t, std::string_view, std::string_view);
        void check_prog_uniform_property_(std::size_t, std::string_view, std::size_t, std::string_view, bool);
        void check_vao_vbo_(std::size_t, std::size_t, std::string_view);

        void check_fbo_id_(std::size_t, std::string_view);
        void check_texture_id_(std::size_t, std::string_view);
        void check_draw_range_(std::size_t, std::size_t, std::size_t, std::size_t, std::string_view);

        void get_program_all_uniforms_(std::size_t);

        int teximage_internalformat_(nre::gpu::TEXTURE_TYPE);
        int teximage_format_(nre::gpu::TEXTURE_TYPE);
        int teximage_type_(nre::gpu::TEXTURE_TYPE);

        // this is useful for preventing OpenGL glBind* command repetitions
        GLuint active_vbo_{0};
        GLuint active_vao_{0};
        GLuint active_ubo_{0};
        GLuint active_prog_{0};
        // every VAO in OpenGL stores its Index Buffer
        std::unordered_map<GLuint, GLuint> vao_ibos_;

        uint32_t                 x_{0};
        uint32_t                 y_{0};
        nre::gpu::SHADER_BACKEND backend_;
        int                      major_{3};
        int                      minor_{3};
        bool                     sanity_checks_{true};
    };
}; }; // namespace nre::gl3


#endif

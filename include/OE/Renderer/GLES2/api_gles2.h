#ifndef NRE_GLES2_API_H_INCLUDED
#define NRE_GLES2_API_H_INCLUDED


#include <OE/Renderer/GLES2/shaders_gles2.h>
#include <OE/Renderer/api_gpu.h>
#include <OE/types/libs_oe.h>

namespace nre { namespace gles2 {
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
        GLenum                 type_{GL_UNSIGNED_INT};
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

    struct vertex_array_t {
        GLuint                                     handle;
        std::vector<nre::gpu::vertex_layout_input> layout;
    };

    struct program_uniform_state_t {
        GLint  slot{0};
        GLenum type{GL_FLOAT}; // unused in Uniform blocks
        size_t size{0};        // unused in Uniform Blocks
        bool   checked{false}; // used to speed up sanity checks
    };

    struct program_data_t {
        bool                                                     checked{false};
        GLuint                                                   handle{0};
        std::unordered_map<std::string, program_uniform_state_t> uniforms;
        bool                                                     has_uniform(const std::string&);
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
}; }; // namespace nre::gles2

namespace std {
    template <>
    struct hash<nre::gles2::program_t> {
        auto operator()(const nre::gles2::program_t& xyz) const -> size_t {
            return hash<size_t>{}(xyz.gen_hash());
        }
    };
} // namespace std

namespace nre { namespace gles2 {

    GLenum buffer_use(nre::gpu::BUFFER_USAGE usage);

    class api_t {
    public:
        api_t(nre::gpu::info_struct&);
        ~api_t();

        void update(uint32_t, uint32_t);

        void destroy();

        std::string get_rendering_api();

        std::size_t new_vertex_buffer();
        std::size_t new_vertex_layout();
        std::size_t new_index_buffer();
        std::size_t new_program();
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

        void set_program_texture_slot(std::size_t, const std::string&, int);
        void set_program_uniform_data(std::size_t, const std::string&, uint32_t);
        void set_program_uniform_data(std::size_t, const std::string&, float);
        void set_program_uniform_data(std::size_t, const std::string&, const std::vector<float>&);
        int  get_program_uniform_slot(std::size_t, const std::string&);

        void set_vertex_layout_format(std::size_t, std::vector<nre::gpu::vertex_layout_input>);
        void delete_vertex_layout(std::size_t);

        void set_texture_format(std::size_t, nre::gpu::TEXTURE_TYPE, nre::gpu::TEXTURE_FILTER, uint32_t, uint32_t, int);
        void set_framebuffer_texture(std::size_t, std::size_t, int);
        void set_texture_slot(std::size_t, int);
        void delete_texture(std::size_t);

        void copy_framebuffer(std::size_t, std::size_t, nre::gpu::FRAMEBUFFER_COPY);
        void use_framebuffer(std::size_t);
        void clear_framebuffer(std::size_t, nre::gpu::FRAMEBUFFER_COPY, float);
        void delete_framebuffer(std::size_t);

        void set_program_vs(std::size_t, nre::gpu::vertex_shader_t);
        void set_program_fs(std::size_t, nre::gpu::pixel_shader_t);

        void set_program_vs(std::size_t, std::string);
        // void set_program_gs(std::size_t, FE_GPU_Shader);
        void set_program_fs(std::size_t, std::string);
        // void set_program_tcs(std::size_t, FE_GPU_Shader);
        // void set_program_tes(std::size_t, FE_GPU_Shader);
        void setup_program(std::size_t);
        void delete_program(std::size_t);

        void draw(nre::gpu::draw_call);

        void set_render_mode(nre::gpu::RENDERMODE);
        void use_wireframe(bool);

    protected:
        std::size_t cur_rbo_{0};
        std::size_t cur_vbo_{0};
        std::size_t cur_ibo_{0};
        std::size_t cur_vao_{0};
        std::size_t cur_fbo_{0};
        std::size_t cur_texture_{0};
        std::size_t cur_prog_{0};

        std::size_t enabled_vao_attribs_{0};

        std::unordered_map<std::size_t, renderbuffer_t>  rbos_;
        std::unordered_map<std::size_t, vertex_buffer_t> vbos_;
        std::unordered_map<std::size_t, index_buffer_t>  ibos_;
        std::unordered_map<std::size_t, vertex_array_t>  vaos_;
        std::unordered_map<std::size_t, framebuffer_t>   fbos_;
        std::unordered_map<std::size_t, texture_t>       textures_;
        std::unordered_map<std::size_t, program_t>       progs_;

        std::size_t get_vao_size(std::size_t);

        std::unordered_map<nre::gpu::vertex_shader_t, GLuint> vs_db_;
        std::unordered_map<nre::gpu::pixel_shader_t, GLuint>  fs_db_;
        std::unordered_map<program_t, program_data_t>         prog_db_;

    private:
        void check_rbo_id(std::size_t, const std::string&);
        void check_vbo_id(std::size_t, const std::string&);
        void check_ibo_id(std::size_t, const std::string&);

        void check_vbo_offset_length(std::size_t, std::size_t, const std::string&);
        void check_ibo_offset_length(std::size_t, std::size_t, const std::string&);

        void check_vao_id(std::size_t, const std::string&);
        void check_prog_id(std::size_t, const std::string&);
        void check_prog_complete(std::size_t, const std::string&);

        void check_prog_uniform_block(std::size_t, const std::string&, const std::string&);
        void check_prog_uniform(std::size_t, const std::string&, const std::string&);
        void check_prog_uniform_property(std::size_t, const std::string&, std::size_t, const std::string&, bool);
        void check_vao_vbo(std::size_t, std::size_t, const std::string&);

        void check_fbo_id(std::size_t, const std::string&);
        void check_texture_id(std::size_t, const std::string&);
        void check_draw_range(std::size_t, std::size_t, std::size_t, std::size_t, const std::string&);

        void get_program_all_uniforms(std::size_t);

        int teximage_internalformat(nre::gpu::TEXTURE_TYPE);
        int teximage_format(nre::gpu::TEXTURE_TYPE);
        int teximage_textype(nre::gpu::TEXTURE_TYPE);

        // this is useful for preventing OpenGL glBind* command repetitions
        GLuint active_vbo_{0};
        GLuint active_vao_{0};
        GLuint active_prog_{0};
        // every VAO in OpenGL stores its Index Buffer
        std::unordered_map<GLuint, GLuint> vao_ibos_;

        /// opengl es extensions
        bool has_oes_packed_depth_stencil_{false};
        bool has_oes_depth24_{false};
        bool has_oes_depth_texture_{false}; // webgl_depth_texture
        bool has_ext_color_buffer_float_{false};
        bool has_ext_color_buffer_half_float_{false};
        bool has_oes_element_index_uint_{false};
        bool has_oes_texture_float_{false};
        bool has_oes_texture_float_linear_{false};
        bool has_oes_texture_half_float_{false};
        bool has_oes_texture_half_float_linear_{false};
        bool has_ext_blend_minmax_{false};
        bool has_ext_texture_filter_anisotropic_{false};
        bool has_oes_texture_npot_{false};
        bool has_ext_texture_compression_s3tc_{false}; // WEBGL_compressed_texture_s3tc
        bool has_ext_srgb_{false};

        uint32_t x_{0};
        uint32_t y_{0};
        int      major_{2};
        int      minor_{0};
    };
}; }; // namespace nre::gles2
#endif

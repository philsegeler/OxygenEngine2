#ifndef NRE_GPU_API_H_INCLUDED
#define NRE_GPU_API_H_INCLUDED

#include <types/OE_TypesBase.h>
#include <Renderer/NRE_GPU_Shaders.h>
#include <Renderer/NRE_Exceptions.h>
#include <atomic>

/** platform-specific namespaces
* -OpenGL ES 3.0/ OpenGL 3.2
* -OpenGL 4.x
* -OpenGL 4.5 + GL_ARB_bindless_texture (UPDATE 2020: OpenGL 3.3/ OpenGL ES 3.0 + extensions
* -Direct3D 9 (maybe, 5% chance to happen UPDATE 2020: ZERO chance to happen) 
*/

namespace nre{ namespace gpu{
    
    struct vertex_layout_input{
        std::size_t vertex_buffer;
        std::size_t offset;
        unsigned int amount;
        unsigned int stride;
    
        vertex_layout_input();
        vertex_layout_input(std::size_t, std::size_t, unsigned int, unsigned int);
    };

    enum BUFFER_USAGE{
        DYNAMIC,
        STATIC,
        STREAM,
        NO_REALLOCATION
    };

    enum RENDERMODE{
        REGULAR_BACKFACE,
        REGULAR_FRONTFACE,
        LIGHT_PREPASS,
        LIGHT_AFTERPASS,
        LIGHT_PREPASS_2,
        LIGHT_AFTERPASS_RG,
        LIGHT_AFTERPASS_BA,
        Z_PREPASS_BACKFACE,
        AFTERPREPASS_BACKFACE,
        REGULAR_BOTH,
        TRANSPARENT_BACKFACE,
        FULLSCREEN_QUAD
    };

    enum TEXTURE_TYPE{
        FLOAT,
        RGBA,
        RGB10_A2,
        RGBA16F,
        SRGBA,
        RGBA_U16,
        RGBA_U8,
        DEPTHSTENCIL
    };

    enum TEXTURE_FILTER{
        LINEAR,
        NEAREST
    };

    enum FRAMEBUFFER_COPY{
        FBO_COLOR,
        FBO_DEPTHSTENCIL,
        FBO_COLORSTENCIL,
        FBO_ALL
    };
    
    struct draw_call{
        std::size_t vertex_layout{0};
        std::size_t index_buf{0};
        std::size_t program{0};
        int offset{0};
        int amount{0};
    };
    
    
    struct info_struct{
        bool has_init{false};
        
        bool has_indexed_ranged_draws{true};
        bool has_uniform_buffers{true};
        bool has_occlusion_query{true};
        bool has_cubemap_array{true};
        bool has_shadow_sampler{true};
        bool has_instancing{true};
        bool has_depth_textures{true};
        bool has_float_textures{true};
        bool has_float_render_targets{true};
        bool has_depth_24{true};
        SHADER_BACKEND underlying_api{UNDEFINED};
        int major{2};
        int minor{0};
    };
    
    //////////////////////
    //  all variables here
    extern void* api;
    extern info_struct backend_info;
    extern std::atomic<bool>  use_wireframe;
    
    extern uint32_t x;
    extern uint32_t y;
    //////////////////////
    
    info_struct get_backend_info();
    SHADER_BACKEND get_api();
    
    std::string get_underlying_api_name();
    
    
    std::size_t new_vertex_buf();
    std::size_t new_program();
    std::size_t new_vertex_layout();
    std::size_t new_index_buf();
    std::size_t new_uniform_buf();
    std::size_t new_framebuffer();
    std::size_t new_texture();
    std::size_t new_renderbuffer();
    
    bool init(SHADER_BACKEND,  int,  int);
    void update(uint32_t, uint32_t);
    void destroy();
    
    void set_renderbuffer_mode(std::size_t, TEXTURE_TYPE, int, int);
    void set_framebuffer_renderbuffer(std::size_t, std::size_t, int);;
    
    void set_vertex_buf_memory(std::size_t, std::size_t, BUFFER_USAGE);
    void set_vertex_buf_data(std::size_t, const std::vector<float>&, std::size_t);
    void set_vertex_buf_memory_and_data(std::size_t, const std::vector<float>&, BUFFER_USAGE);
    void del_vertex_buf(std::size_t);
    
    void set_index_buf_memory(std::size_t, std::size_t, BUFFER_USAGE);
    void set_index_buf_data(std::size_t, const std::vector<uint32_t>&, std::size_t);
    void set_index_buf_memory_and_data(std::size_t, const std::vector<uint32_t>&, BUFFER_USAGE);
    void del_index_buf(std::size_t);
    
    void set_uniform_buf_memory(std::size_t, std::size_t, BUFFER_USAGE);
    void set_uniform_buf_data(std::size_t, const std::vector<float>&, std::size_t);
    void set_uniform_buf_data(std::size_t, const std::vector<uint32_t>&, std::size_t);
    void set_uniform_buf_memory_and_data(std::size_t, const std::vector<uint32_t>&, BUFFER_USAGE);
    void set_uniform_buf_memory_and_data(std::size_t, const std::vector<float>&, BUFFER_USAGE);
    void del_uniform_buf(std::size_t);
    
    void set_uniform_buf_state(std::size_t, std::size_t, int, std::size_t, std::size_t);
    
    void set_program_uniform_buf_slot(std::size_t, std::string, int);
    int  get_program_uniform_buf_slot(std::size_t, std::string);
    
    void set_program_texture_slot(std::size_t, std::string, int);
    void set_program_uniform_data(std::size_t, std::string, uint32_t);
    void set_program_uniform_data(std::size_t, std::string, float);
    void set_program_uniform_data(std::size_t, std::string, const std::vector<uint32_t>&);
    void set_program_uniform_data(std::size_t, std::string, const std::vector<float>&);
    int  get_program_uniform_slot(std::size_t, std::string);
    
    void set_vertex_layout_format(std::size_t, std::vector<vertex_layout_input>);
    void del_vertex_layout(std::size_t);
    
    void set_texture_format(std::size_t, TEXTURE_TYPE, TEXTURE_FILTER, uint32_t, uint32_t, int);
    void set_texture_slot(std::size_t, int);
    void del_texture(std::size_t);
    
    void set_framebuffer_texture(std::size_t, std::size_t, int);
    void copy_framebuffer(std::size_t, std::size_t, FRAMEBUFFER_COPY);
    void use_framebuffer(std::size_t);
    void clear_framebuffer(std::size_t, nre::gpu::FRAMEBUFFER_COPY, float);
    void del_framebuffer(std::size_t);
    
    void set_program_vertex_shader(std::size_t, nre::gpu::vertex_shader);
    void set_program_pixel_shader(std::size_t, nre::gpu::pixel_shader);
    void setup_program(std::size_t);
    void del_program(std::size_t);
    
    void draw(draw_call);
    void draw(std::size_t, std::size_t);
    void draw(std::size_t, std::size_t, std::size_t);
    void draw_instanced(draw_call, int);
    void draw_instanced(std::size_t, std::size_t, std::size_t, int);
    
    void set_render_mode(nre::gpu::RENDERMODE);
    
};
};


#endif 

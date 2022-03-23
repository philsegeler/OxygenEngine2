#ifndef NRE_GLES2_API_H_INCLUDED
#define NRE_GLES2_API_H_INCLUDED


#include <Renderer/NRE_GPU_API.h>
#include <types/OE_Libs.h>
#include <Renderer/GLES2/NRE_GLES2_Shaders.h>

struct NRE_GLES2_RenderBuffer{
    GLuint handle{0};
    nre::gpu::TEXTURE_TYPE type;
    int x{0};
    int y{0};
    
    bool hasNotChanged(nre::gpu::TEXTURE_TYPE, int, int);
};

struct NRE_GLES2_VertexBuffer{
    GLuint                  handle;
    std::size_t             size;
    nre::gpu::BUFFER_USAGE    usage;
};

struct NRE_GLES2_IndexBuffer{
    GLuint                  handle;
    std::size_t             size;
    nre::gpu::BUFFER_USAGE    usage;
};

struct NRE_GLES2_FrameBuffer{
    GLuint handle;
    //int components{0};
    //bool depth{false};
    //bool stencil{false};
    std::size_t texture{0};
};

struct NRE_GLES2_Texture{
    GLuint handle;
    nre::gpu::TEXTURE_TYPE type;
    nre::gpu::TEXTURE_FILTER filter;
    int x{0};
    int y{0};
    int mipmaps{0};
    
    bool hasNotChanged(nre::gpu::TEXTURE_TYPE, nre::gpu::TEXTURE_FILTER, int, int, int);
};

struct NRE_GLES2_VertexArray{
    GLuint                                  handle;
    std::vector<nre::gpu::vertex_layout_input>  layout;
};

struct NRE_GLES2_ProgramUniformState{
    std::string name;
    GLint slot{0};
    GLenum type{GL_FLOAT}; //unused in Uniform blocks
    size_t size{0}; //unused in Uniform Blocks
};

struct NRE_GLES2_Program{
    
    nre::gpu::vertex_shader vs;
    GLuint vs_handle{0};
    bool vs_setup{false};
    
    nre::gpu::pixel_shader fs;
    GLuint fs_handle{0};
    bool fs_setup{false};
    
    bool setup{false};
    bool prog_created{false};
    GLuint handle{0};
    
    // this is needed for it to be in an std::set
    bool operator< (const NRE_GLES2_Program&) const;
};

struct NRE_GLES2_ProgramData{
    GLuint handle{0};
    std::vector<NRE_GLES2_ProgramUniformState> uniforms;
    std::size_t hasUniform(std::string);
};

GLenum NRE2GLES2_BufferUse(nre::gpu::BUFFER_USAGE usage);

class NRE_GLES2_API{
public:
    
    NRE_GLES2_API(nre::gpu::info_struct&);
    ~NRE_GLES2_API();
    
    void update(uint32_t, uint32_t);
    
    void destroy();
    
    std::string getRenderingAPI();
    
    std::size_t newVertexBuffer();
    std::size_t newVertexLayout();
    std::size_t newIndexBuffer();
    std::size_t newProgram();
    std::size_t newFrameBuffer();
    std::size_t newTexture();
    std::size_t newRenderBuffer();
    
    void setRenderBufferType(std::size_t, nre::gpu::TEXTURE_TYPE, int, int);
    void setFrameBufferRenderBuffer(std::size_t, std::size_t, int);
    
    void setVertexBufferMemory(std::size_t, std::size_t, nre::gpu::BUFFER_USAGE);
    void setVertexBufferData(std::size_t, const std::vector<float>&, std::size_t);
    void setVertexBufferMemoryData(std::size_t, const std::vector<float>&, nre::gpu::BUFFER_USAGE);
    void deleteVertexBuffer(std::size_t);
    
    void setIndexBufferMemory(std::size_t, std::size_t, nre::gpu::BUFFER_USAGE);
    void setIndexBufferData(std::size_t, const std::vector<uint32_t>&, std::size_t);
    void setIndexBufferMemoryData(std::size_t, const std::vector<uint32_t>&, nre::gpu::BUFFER_USAGE);
    void deleteIndexBuffer(std::size_t);
    
    void setProgramTextureSlot(std::size_t, std::string, int);
    void setProgramUniformData(std::size_t, std::string, uint32_t);
    void setProgramUniformData(std::size_t, std::string, float);
    void setProgramUniformData(std::size_t, std::string, const std::vector<float>&);
    int  getProgramUniformSlot(std::size_t, std::string);
    
    void setVertexLayoutFormat(std::size_t, std::vector<nre::gpu::vertex_layout_input>);
    void deleteVertexLayout(std::size_t);
    
    void setTextureFormat(std::size_t, nre::gpu::TEXTURE_TYPE, nre::gpu::TEXTURE_FILTER, uint32_t, uint32_t, int);
    void setFrameBufferTexture(std::size_t, std::size_t, int);
    void setTextureSlot(std::size_t, int);
    void deleteTexture(std::size_t);
    
    void copyFrameBuffer(std::size_t, std::size_t, nre::gpu::FRAMEBUFFER_COPY);
    void useFrameBuffer(std::size_t);
    void clearFrameBuffer(std::size_t, nre::gpu::FRAMEBUFFER_COPY, float);
    void deleteFrameBuffer(std::size_t);
    
    void setProgramVS(std::size_t, nre::gpu::vertex_shader);
    void setProgramFS(std::size_t, nre::gpu::pixel_shader);
    
    void setProgramVS(std::size_t, std::string);
    //void setProgramGS(std::size_t, FE_GPU_Shader);
    void setProgramFS(std::size_t, std::string);
    //void setProgramTCS(std::size_t, FE_GPU_Shader);
    //void setProgramTES(std::size_t, FE_GPU_Shader);
    void setupProgram(std::size_t);
    void deleteProgram(std::size_t);
    
    void draw(nre::gpu::draw_call);
    
    void setRenderMode(nre::gpu::RENDERMODE);
    
protected:
    
    std::size_t cur_rbo{0};
    std::size_t cur_vbo{0};
    std::size_t cur_ibo{0};
    std::size_t cur_vao{0};
    std::size_t cur_fbo{0};
    std::size_t cur_texture{0};
    std::size_t cur_prog{0};
    
    std::size_t enabled_vao_attribs{0};
    
    std::unordered_map<std::size_t, NRE_GLES2_RenderBuffer>     rbos;
    std::unordered_map<std::size_t, NRE_GLES2_VertexBuffer>     vbos;
    std::unordered_map<std::size_t, NRE_GLES2_IndexBuffer>      ibos;
    std::unordered_map<std::size_t, NRE_GLES2_VertexArray>      vaos;
    std::unordered_map<std::size_t, NRE_GLES2_FrameBuffer>      fbos;
    std::unordered_map<std::size_t, NRE_GLES2_Texture>          textures;
    std::unordered_map<std::size_t, NRE_GLES2_Program>          progs;
    
    std::size_t getVAOSize(std::size_t);
    
    std::map<nre::gpu::vertex_shader,  GLuint> vs_db;
    std::map<nre::gpu::pixel_shader,   GLuint> fs_db;
    std::map<NRE_GLES2_Program,       NRE_GLES2_ProgramData> prog_db;
    
private:
    
    void check_rbo_id_(std::size_t, const std::string&);
    void check_vbo_id_(std::size_t, const std::string&);
    void check_ibo_id_(std::size_t, const std::string&);
    
    void check_vbo_offset_length_(std::size_t, std::size_t, const std::string&);
    void check_ibo_offset_length_(std::size_t, std::size_t, const std::string&);
    
    void check_vao_id_(std::size_t, const std::string&);
    void check_prog_id_(std::size_t, const std::string&);
    void check_prog_complete_(std::size_t, const std::string&);
    
    void check_prog_uniform_block_(std::size_t, const std::string&, const std::string&);
    void check_prog_uniform_(std::size_t, const std::string&, const std::string&);
    void check_prog_uniform_property_(std::size_t, const std::string&, std::size_t, const std::string&, bool);
    void check_vao_vbo_(std::size_t, std::size_t, const std::string&);
    
    void check_fbo_id_(std::size_t, const std::string&);
    void check_texture_id_(std::size_t, const std::string&);
    void check_draw_range_(std::size_t, std::size_t, std::size_t, std::size_t, const std::string&);
    
    void get_program_all_uniforms_(std::size_t);
    
    int teximage_internalformat_(nre::gpu::TEXTURE_TYPE);
    int teximage_format_(nre::gpu::TEXTURE_TYPE);
    int teximage_type_(nre::gpu::TEXTURE_TYPE);
    
    // this is useful for preventing OpenGL glBind* command repetitions
    GLuint active_vbo_{0};
    GLuint active_vao_{0};
    GLuint active_prog_{0};
    // every VAO in OpenGL stores its Index Buffer
    std::unordered_map<GLuint, GLuint> vao_ibos_;
    
    /// opengl es extensions
    bool has_oes_packed_depth_stencil{false};
    bool has_oes_depth24{false};
    bool has_oes_depth_texture{false}; //webgl_depth_texture
    bool has_ext_color_buffer_float{false};
    bool has_ext_color_buffer_half_float{false};
    bool has_oes_element_index_uint{false};
    bool has_oes_texture_float{false};
    bool has_oes_texture_float_linear{false};
    bool has_oes_texture_half_float{false};
    bool has_oes_texture_half_float_linear{false};
    bool has_ext_blend_minmax{false};
    bool has_ext_texture_filter_anisotropic{false};
    bool has_oes_texture_npot{false};
    bool has_ext_texture_compression_s3tc{false}; //WEBGL_compressed_texture_s3tc
    bool has_ext_srgb{false};
    
}; 

#endif

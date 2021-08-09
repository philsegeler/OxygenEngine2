#ifndef NRE_EXCEPTIONS_H
#define NRE_EXCEPTIONS_H

#include <types/OE_TypesBase.h>

namespace nre {
    class not_initialized: public oe::renderer_error {
        public:
            
            not_initialized(){
                name_ = "nre::not_initialized";
            }
            std::string what() const throw() {
                return "'renderer->init()' must be called before r";
            };
    };
    
    namespace gpu {
    
        class invalid_render_buffer: public oe::renderer_error {
            public:
                
                invalid_render_buffer(std::size_t id, const std::string &func){
                    name_ = "nre::gpu::invalid_render_buffer";
                    id_ = std::to_string(id);
                    func_ = func;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with invalid render buffer ID: '" + id_ + "'.";
                };
                
                std::string id_;
                std::string func_;
        };
        
        class invalid_vertex_buffer: public oe::renderer_error {
            public:
                
                invalid_vertex_buffer(std::size_t id, const std::string &func){
                    name_ = "nre::gpu::invalid_vertex_buffer";
                    id_ = std::to_string(id);
                    func_ = func;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with invalid vertex buffer ID: '" + id_ + "'.";
                };
                
                std::string id_;
                std::string func_;
        };
        
        class invalid_index_buffer: public oe::renderer_error {
            public:
                
                invalid_index_buffer(std::size_t id, const std::string &func){
                    name_ = "nre::gpu::invalid_index_buffer";
                    id_ = std::to_string(id);
                    func_ = func;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with invalid index buffer ID: '" + id_ + "'.";
                };
                
                std::string id_;
                std::string func_;
        };
        
        class invalid_uniform_buffer: public oe::renderer_error {
            public:
                
                invalid_uniform_buffer(std::size_t id, const std::string &func){
                    name_ = "nre::gpu::invalid_uniform_buffer";
                    id_ = std::to_string(id);
                    func_ = func;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with invalid uniform buffer ID: '" + id_ + "'.";
                };
                
                std::string id_;
                std::string func_;
        };
        
        class invalid_vertex_layout: public oe::renderer_error {
            public:
                
                invalid_vertex_layout(std::size_t id, const std::string &func){
                    name_ = "nre::gpu::invalid_vertex_layout";
                    id_ = std::to_string(id);
                    func_ = func;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with invalid vertex layout ID: '" + id_ + "'.";
                };
                
                std::string id_;
                std::string func_;
        };
        
        class invalid_framebuffer: public oe::renderer_error {
            public:
                
                invalid_framebuffer(std::size_t id, const std::string &func){
                    name_ = "nre::gpu::invalid_framebuffer";
                    id_ = std::to_string(id);
                    func_ = func;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with invalid framebuffer ID: '" + id_ + "'.";
                };
                
                std::string id_;
                std::string func_;
        };
        
        class invalid_texture: public oe::renderer_error {
            public:
                
                invalid_texture(std::size_t id, const std::string &func){
                    name_ = "nre::gpu::invalid_texture";
                    id_ = std::to_string(id);
                    func_ = func;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with invalid texture ID: '" + id_ + "'.";
                };
                
                std::string id_;
                std::string func_;
        };
        
        class invalid_vertex_layout_buffer: public oe::renderer_error {
            public:
                
                invalid_vertex_layout_buffer(std::size_t id, std::size_t vbo, const std::string &func){
                    name_ = "nre::gpu::invalid_vertex_layout_buffer";
                    id_ = std::to_string(id);
                    func_ = func;
                    vbo_ = vbo;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with vertex layout ID: '" + id_ + "'. Invalid vertex buffer ID: '"  + vbo_ + "'.";
                };
                
                std::string id_;
                std::string func_;
                std::string vbo_;
        };
        
        class invalid_program_id: public oe::renderer_error {
            public:
                
                invalid_program_id(std::size_t id, const std::string &func){
                    name_ = "nre::gpu::invalid_program_id";
                    id_ = std::to_string(id);
                    func_ = func;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with invalid shader program ID: '" + id_  + "'.";
                };
                
                std::string id_;
                std::string func_;
        };
        
        class invalid_program_uniform: public oe::renderer_error {
            public:
                
                invalid_program_uniform(std::size_t id, std::string uniform, const std::string &func){
                    name_ = "nre::gpu::invalid_program_uniform";
                    id_ = std::to_string(id);
                    func_ = func;
                    uniform_ = uniform;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with shader program ID: '" + id_ + "' having no uniform variable named '" + uniform_ + "'.\n\tNote that it might be optimized out by the shader compiler (for example on Intel GPUs on Windows).";
                };
                
                std::string id_;
                std::string func_;
                std::string uniform_;
        };
        
        class invalid_program_uniform_block: public oe::renderer_error {
            public:
                
                invalid_program_uniform_block(std::size_t id, std::string uniform, const std::string &func){
                    name_ = "nre::gpu::invalid_program_uniform_block";
                    id_ = std::to_string(id);
                    func_ = func;
                    uniform_ = uniform;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with shader program ID: '" + id_ + "' having no uniform BLOCK variable named '" + uniform_ + "'.\n\tNote that it might be optimized out by the shader compiler (for example on Intel GPUs on Windows).";
                };
                
                std::string id_;
                std::string func_;
                std::string uniform_;
        };
        
        
        class invalid_buffer_offset_length: public oe::renderer_error {
            public:
                
                invalid_buffer_offset_length(std::size_t id, std::size_t off_length, std::string buffer, const std::string &func){
                    name_ = "nre::gpu::invalid_buffer_offset_length";
                    id_ = std::to_string(id);
                    func_ = func;
                    off_length_ = std::to_string(off_length);
                    buf_type_ = buffer;
                }
                std::string what() const throw() {
                    return "Calling GPU API function '" + func_ + "' with " + buf_type_ +" buffer ID: '" + id_ + "'.\n\t offset + length = '" + off_length_ + "' goes out of bounds.";
                };
                
                std::string id_;
                std::string func_;
                std::string off_length_;
                std::string buf_type_;
        };
        
        class unimplemented_function: public oe::renderer_error {
            public:
                
                unimplemented_function(const std::string &func, const std::string &api_name, const std::string& extra_info){
                    name_ = "nre::gpu::unimplemented_function";
                    func_ = func;
                    api_ = api_name;
                    info_ = extra_info;
                }
                std::string what() const throw() {
                    return "GPU API function '" + func_ + "' is not implemented in the API: '" + api_  + "'. " + info_;
                };
                
                std::string id_;
                std::string func_;
                std::string api_;
                std::string info_;
        };
        
        class unsupported_texture: public oe::renderer_error {
            public:
                
                unsupported_texture(const std::string &func, const std::string &api_name, const std::string& tex_type_in){
                    name_ = "nre::gpu::unsupported_texture";
                    func_ = func;
                    api_ = api_name;
                    tex_type_ = tex_type_in;
                }
                std::string what() const throw() {
                    return "GPU API function '" + func_ + "' in the API: '" + api_  + "' does not support '" + tex_type_ + "' textures. Remember to query support for a particular feature before using it.";
                };
                
                std::string id_;
                std::string func_;
                std::string api_;
                std::string tex_type_;
        };
        
    };
    
    class incomplete_viewport: public oe::renderer_error {
        public:
            
            incomplete_viewport(std::size_t id){
                name_ = "nre::incomplete_viewport";
                id_ = std::to_string(id);
            }
            std::string what() const throw() {
                return "loaded_viewport with ID: '" + id_ + "' is incomplete. No cameras are defined.";
            };
            
            std::string id_;
            std::string func_;
    };
    
    class unsupported_viewport: public oe::renderer_error {
        public:
            
            unsupported_viewport(std::size_t id, std::string msg){
                name_ = "nre::unsupported_viewport";
                id_ = std::to_string(id);
                msg_ = msg;
            }
            std::string what() const throw() {
                return "loaded_viewport with ID: '" + id_ + "' is not possible to display. " + msg_ + ".";
            };
            
            std::string id_;
            std::string func_;
            std::string msg_;
    };
    
};

#endif

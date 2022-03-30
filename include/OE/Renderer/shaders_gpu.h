#ifndef NRE_GPU_SHADERS_H
#define NRE_GPU_SHADERS_H

#define NRE_Shader(src) #src

#include <OE/types/base_types.h>

namespace nre { namespace gpu {

    // possibility to extend with HLSL/Metal etc.
    enum SHADER_BACKEND : int { UNDEFINED, GL, GLES, GLES2 };

    enum VS_TYPE {
        VS_UNDEFINED,
        VS_Z_PREPASS,
        VS_REGULAR,
        VS_BOUNDING_BOX,
        VS_BOUNDING_SPHERE,
        VS_LIGHT,
    };

    enum FS_TYPE {
        FS_UNDEFINED,
        FS_SIMPLE,
        FS_GAMMA,
        FS_MATERIAL,
        FS_NORMALS,
        FS_SIMPLE_LIGHT,
        FS_DIR_LIGHT,
        FS_INDIR_LIGHTS,
        FS_LIGHT_INDEX
    };

    class shader_base {
    public:
        static std::string shader_prefix;

        static void init(SHADER_BACKEND, int, int);

        virtual ~shader_base();
        virtual std::string gen_shader();
        virtual std::string info();
    };

    class vertex_shader : public shader_base {
    public:
        vertex_shader();
        ~vertex_shader();

        bool operator<(const vertex_shader&) const;
        bool operator==(const vertex_shader&) const;

        std::string gen_shader();
        std::string info();

        bool        fullscreenQuad{false};
        std::size_t num_of_uvs{0};
        VS_TYPE     type{VS_UNDEFINED};
    };

    class pixel_shader : public shader_base {
    public:
        pixel_shader();
        ~pixel_shader();

        bool operator<(const pixel_shader&) const;

        std::string gen_shader();
        std::string info();

        std::size_t num_of_uvs{0};
        FS_TYPE     type{FS_UNDEFINED};
    };

}; }; // namespace nre::gpu
#endif

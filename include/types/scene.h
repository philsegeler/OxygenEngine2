#ifndef OE_SCENE_H
#define OE_SCENE_H

#include <types/camera.h>
#include <types/light.h>
#include <types/material.h>
#include <types/mesh.h>
#include <types/tcm.h>
#include <types/texture.h>

/* OE_Scene is a collection of objects (meshes, lights, cameras, etc.)
 * textures, materials, TCMs etc.
 */

class OE_Scene : public OE_THREAD_SAFETY_OBJECT, public CSL_WriterBase {

    friend class CSL_Interpreter;

public:
    static std::atomic<std::size_t> current_id;
    std::size_t                     id;

    OE_Scene();
    OE_Scene(const std::string&);
    ~OE_Scene();

    std::string to_str() const;

    // protected:

    std::set<std::size_t> objects;
    std::set<std::size_t> materials;
    std::set<std::size_t> textures;
    std::set<std::size_t> texture_CMs;
};



#endif

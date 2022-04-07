#ifndef OE_DUMMYCLASSES_H
#define OE_DUMMYCLASSES_H

#include <OE/Events/event_handler.h>
#include <OE/Renderer/api_gpu.h>
#include <OE/task.h>
#include <OE/types/base_types.h>
#include <OE/types/world.h>

namespace oe {

    enum OS : int { OS_UNDEFINED = 0, OS_LINUX = 1, OS_WINDOWS = 2, OS_ANDROID = 3, OS_WEB = 4 };

    enum WINSYS : int { WINSYS_NONE = 0, WINSYS_SDL = 1, WINSYS_SOMETHING_ELSE = 2 };

    enum RENDERER_SHADING_MODE : int {
        RENDERER_NORMALS_SHADING        = 0,
        RENDERER_NO_LIGHTS_SHADING      = 1,
        RENDERER_DIR_LIGHTS_SHADING     = 2,
        RENDERER_INDEXED_LIGHTS_SHADING = 3,
        RENDERER_REGULAR_SHADING        = 4
    };

    /** This a dummy class aimed to be used as a base class for windowing systems
     * Plus relevant structs for getting info in and out
     */

    // initial parameters
    struct winsys_init_info {
        nre::gpu::SHADER_BACKEND requested_backend{nre::gpu::GLES2};
    };

    // dynamic parameters
    struct winsys_update_info {
        std::string title;
        int         res_x{640};
        int         res_y{480};
        bool        use_fullscreen{false};
        bool        vsync{true};
        bool        mouse_locked{true};
    };

    // output to the renderer
    struct winsys_output {
        int                      res_x{640};
        int                      res_y{480};
        nre::gpu::SHADER_BACKEND backend{nre::gpu::GLES2};
        int                      major{2};
        int                      minor{0};
        bool                     restart_renderer{false};
        bool                     done{false};
        int                      dpi{96};
    };

    class winsys_base_t : public OE_THREAD_SAFETY_OBJECT {
    public:
        winsys_base_t();
        virtual ~winsys_base_t();


        virtual bool init(int, int, std::string, bool, bool, void*);
        virtual bool update();

        virtual bool getMouseLockedState();
        virtual void lockMouse();
        virtual void unlockMouse();

        virtual bool updateEvents();
        virtual void destroy();

        int  resolution_x{0};
        int  resolution_y{0};
        int  dpi{96};
        bool vsync{true};

        OS     os{OS_UNDEFINED};
        WINSYS winsys{WINSYS_NONE};

        // For different OpenGL versions
        std::string title;
        bool        fullscreen{false};
        int         major{0};
        int         minor{0};
        bool        isES{false};

        bool mouse_locked{false};

        std::atomic<bool> reset_renderer{false};
        std::atomic<bool> restart_renderer{false};

        // The global event handler is here and must be initialized in all sub classes
        oe::event_handler_t event_handler_;
    };

    /** This is a dummy class aimed to be a base class for
     *  a renderer
     * plus relevant structs for getting info in and out
     */
    // initial parameters
    struct renderer_init_info {
        bool some_var{false};
    };

    // dynamic parameters
    struct renderer_update_info {
        bool                  use_hdr{false};
        bool                  use_wireframe{false};
        bool                  use_light_indexed_rendering{false};
        bool                  render_bounding_boxes{false};
        bool                  render_bounding_spheres{false};
        bool                  use_z_prepass{true};
        RENDERER_SHADING_MODE shading_mode{RENDERER_REGULAR_SHADING};
    };

    class renderer_base_t : public OE_THREAD_SAFETY_OBJECT {
    public:
        renderer_base_t();
        virtual ~renderer_base_t();

        virtual bool init();
        virtual bool updateSingleThread();
        virtual bool updateData();

        virtual bool updateMultiThread(OE_Task*, int);
        virtual void destroy();

        bool                      isMultiThreaded{false};
        std::shared_ptr<OE_World> world{nullptr};
        winsys_base_t*            screen{nullptr};
        std::string               name{"default"};

        RENDERER_SHADING_MODE shading_mode{RENDERER_REGULAR_SHADING};
        std::atomic<bool>     use_wireframe{false};
        std::atomic<bool>     render_bounding_boxes{false};
        std::atomic<bool>     render_bounding_spheres{false};
        std::atomic<bool>     use_HDR{false};
    };

    /** This is a dummy class aimed to be a base class for
     *  a physics engine
     * plus relevant structs for getting info in and out
     */
    // initial parameters
    struct physics_init_info {
        bool some_var{false};
    };

    // dynamic parameters
    struct physics_update_info {
        float gravity{-9.81f};
    };

    class physics_base_t : public OE_THREAD_SAFETY_OBJECT {
    public:
        physics_base_t();
        virtual ~physics_base_t();

        virtual bool init();

        virtual bool updateMultiThread(OE_Task*, int);
        virtual void destroy();

        bool                      isMultiThreaded{false};
        std::shared_ptr<OE_World> world{nullptr};
        // You do not actually need this since you can use API functions directly in your
        // Physics engine .cpp :))
        // OE_EventHandler*    handler{nullptr};
        std::string name{"default"};
    };

    /** This is a dummy class aimed to be a base class for
     *  a netwotking manager
     * plus relevant structs for getting info in and out
     */

    class networking_base_t : public OE_THREAD_SAFETY_OBJECT {
    public:
        networking_base_t();
        virtual ~networking_base_t();

        virtual void init();
        virtual int  execute(OE_Task);
        virtual void destroy();

        std::atomic<bool> done{false};
        std::string       name{"default"};
    };
}; // namespace oe
#endif

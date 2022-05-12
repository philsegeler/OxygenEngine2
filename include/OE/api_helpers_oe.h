#ifndef OE_API_HELPER_INCLUDED
#define OE_API_HELPER_INCLUDED

#include <OE/task_manager.h>

namespace OE_API_Helpers {

    oe::task_action load_world(const oe::task_info_t, std::string);

    oe::task_action manage_mouse(const oe::task_info_t);

    void checkIfInit();
}; // namespace OE_API_Helpers

#endif

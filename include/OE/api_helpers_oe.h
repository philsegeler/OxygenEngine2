#ifndef OE_API_HELPER_INCLUDED
#define OE_API_HELPER_INCLUDED

#include <OE/task_manager.h>

namespace OE_API_Helpers {

    int load_world(const oe::task_info_t, std::string);

    int manage_mouse(const oe::task_info_t, std::size_t);

    void checkIfInit();
}; // namespace OE_API_Helpers

#endif

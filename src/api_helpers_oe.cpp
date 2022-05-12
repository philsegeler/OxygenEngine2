#include <OE/Carbon/interpreter.h>
#include <OE/api_helpers_oe.h>
#include <OE/api_oe.h>

using namespace oe;
using namespace OE_API_Helpers;
using namespace std;

oe::task_action OE_API_Helpers::load_world(const oe::task_info_t task, string filename) {

    csl::interpret_file(filename);

    oe::broadcast_event("loaded-" + filename);

    return oe::task_action::discard;
}

oe::task_action OE_API_Helpers::manage_mouse(const oe::task_info_t task) {

    if (oe::get_event_name(task.get_id()) == "mouse-lock") {
        OE_Main->window->lock_mouse();
    }
    else if (oe::get_event_name(task.get_id()) == "mouse-unlock") {
        OE_Main->window->unlock_mouse();
    }
    return oe::task_action::keep;
}

void OE_API_Helpers::checkIfInit() {

    if (OE_Main != nullptr) {
        return;
    }
    else {
        throw oe::not_initialized();
    }
}

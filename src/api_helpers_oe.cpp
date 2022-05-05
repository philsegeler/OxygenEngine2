#include <OE/api_helpers_oe.h>
#include <OE/api_oe.h>


using namespace oe;
using namespace OE_API_Helpers;
using namespace std;

int OE_API_Helpers::load_world(OE_Task task, string filename) {

    csl::interpret_file(filename);

    oe::broadcast_event("loaded-" + filename);

    return 0;
}

int OE_API_Helpers::manage_mouse(OE_Task task, std::size_t event_id) {

    if (oe::get_event_name(event_id) == "mouse-lock") {
        OE_Main->window->lock_mouse();
    }
    else if (oe::get_event_name(event_id) == "mouse-unlock") {
        OE_Main->window->unlock_mouse();
    }
    return 0;
}

void OE_API_Helpers::checkIfInit() {

    if (OE_Main != nullptr) {
        return;
    }
    else {
        throw oe::not_initialized();
    }
}

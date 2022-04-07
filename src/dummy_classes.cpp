#include <OE/api_oe.h>
#include <OE/dummy_classes.h>

// Dummy definitions of the class methods
// so that the classes can stand on their own and be used
// for testing purposes

using namespace std;

oe::winsys_base_t::winsys_base_t() {
}

oe::winsys_base_t::~winsys_base_t() {
}

bool oe::winsys_base_t::init(int x, int y, string titlea, bool isFullscreen, bool use_legacy_renderer, void* data) {
    return true;
}

bool oe::winsys_base_t::getMouseLockedState() {
    return mouse_locked;
}
void oe::winsys_base_t::lockMouse() {
}
void oe::winsys_base_t::unlockMouse() {
}

bool oe::winsys_base_t::update() {
    return true;
}

bool oe::winsys_base_t::updateEvents() {
    return true;
}

void oe::winsys_base_t::destroy() {
}


oe::renderer_base_t::renderer_base_t() {
}

oe::renderer_base_t::~renderer_base_t() {
}

bool oe::renderer_base_t::init() {
    return true;
}

bool oe::renderer_base_t::updateSingleThread() {
    return true;
}

bool oe::renderer_base_t::updateData() {
    return true;
}

bool oe::renderer_base_t::updateMultiThread(OE_Task*, int) {
    return true;
}

void oe::renderer_base_t::destroy() {
}


oe::physics_base_t::physics_base_t() {
}

oe::physics_base_t::~physics_base_t() {
}

bool oe::physics_base_t::init() {
    return true;
}

bool oe::physics_base_t::updateMultiThread(OE_Task* task, int thread_num) {
    // throw 5;
    return true;
}

void oe::physics_base_t::destroy() {
    return;
}

oe::networking_base_t::networking_base_t() {
}
oe::networking_base_t::~networking_base_t() {
}

void oe::networking_base_t::init() {
}

int oe::networking_base_t::execute(OE_Task) {

    // int a =0;

    while (!done) {
        // manage networking stuff, use your own threads etc.
        // YOU control the loop. Upon a call on this->destroy() it should stop though
        // use oe::create_event() and oe::broadcastEvent() to communicate with the engine
        // cout << "OE_Networking is running" << ++a << endl;
        oe::pause(10);
    }
    return 0;
}
void oe::networking_base_t::destroy() {
    done = true;
}

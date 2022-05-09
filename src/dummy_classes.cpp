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

oe::winsys_output oe::winsys_base_t::init(oe::winsys_init_info init_info, oe::winsys_update_info update_info) {
    return oe::winsys_output();
}

bool oe::winsys_base_t::is_mouse_locked() {
    return false;
}
void oe::winsys_base_t::lock_mouse() {
}
void oe::winsys_base_t::unlock_mouse() {
}

oe::winsys_output oe::winsys_base_t::update(oe::winsys_update_info update_info) {
    return oe::winsys_output();
}

bool oe::winsys_base_t::update_events() {
    return true;
}

void oe::winsys_base_t::destroy() {
}


oe::renderer_base_t::renderer_base_t(bool mt, std::string name) : use_multithreading_(mt), name_(name) {
}

oe::renderer_base_t::~renderer_base_t() {
}

bool oe::renderer_base_t::init(oe::renderer_init_info init_info, oe::renderer_update_info update_info,
                               oe::winsys_output winsys_info) {
    return true;
}

bool oe::renderer_base_t::update_single_thread(oe::renderer_update_info update_info, oe::winsys_output winsys_info) {
    return true;
}

bool oe::renderer_base_t::update_data(oe::renderer_update_info update_info, oe::winsys_output winsys_info,
                                      bool has_renderer_restarted) {
    return true;
}

bool oe::renderer_base_t::update_multi_thread(const oe::task_info_t, int) {
    return true;
}

void oe::renderer_base_t::destroy() {
}

void oe::renderer_base_t::set_world(std::shared_ptr<OE_World> world_in) {
    lockMutex();
    world_ = world_in;
    unlockMutex();
}
bool oe::renderer_base_t::is_multi_threaded() {
    return use_multithreading_;
}
const std::string oe::renderer_base_t::get_name() {
    return name_;
}

oe::physics_base_t::physics_base_t(std::string name) : name_(name) {
}

oe::physics_base_t::~physics_base_t() {
}

bool oe::physics_base_t::init(physics_init_info params) {
    return true;
}

void oe::physics_base_t::update_info(oe::physics_update_info params) noexcept {
    // update the parameters in one place
}

bool oe::physics_base_t::update_multi_thread(const oe::task_info_t task, int thread_num) {
    // throw 5;
    return true;
}

void oe::physics_base_t::destroy() {
    return;
}

void oe::physics_base_t::set_world(std::shared_ptr<OE_World> world_in) {
    lockMutex();
    world_ = world_in;
    unlockMutex();
}

const std::string oe::physics_base_t::get_name() {
    return name_;
}

oe::networking_base_t::networking_base_t(std::string name) : name_(name) {
}
oe::networking_base_t::~networking_base_t() {
}

void oe::networking_base_t::init(networking_init_info params) {
}

int oe::networking_base_t::execute(const oe::task_info_t) {

    // int a =0;
    while (!done_) {
        // manage networking stuff, use your own threads etc.
        // YOU control the loop. Upon a call on this->destroy() it should stop though
        // use oe::create_event() and oe::broadcastEvent() to communicate with the engine
        // cout << "OE_Networking is running" << ++a << endl;
        oe::pause(10);
    }
    return 0;
}
void oe::networking_base_t::destroy() {
    done_ = true;
}

const std::string oe::networking_base_t::get_name() {
    return name_;
}

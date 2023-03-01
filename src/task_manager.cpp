#include <OE/Renderer/renderer_legacy.h>
#include <OE/Renderer/renderer_main.h>
#include <OE/api_oe.h>
#include <OE/math_oe.h>
#include <OE/task_manager.h>
#include <OE/types/libs_oe.h>
#include <OE/winsys_sdl2.h>

using namespace std;
using namespace oe;

std::atomic<std::size_t> oe::thread_struct::current_id(0);

oe::thread_struct::thread_struct() : id(++oe::thread_struct::current_id) {
}

oe::task_manager_t* oe::thread_data::taskMgr = nullptr;

#ifdef OE_PLATFORM_WEB
std::atomic<bool> oe_threads_ready_to_start = false;
#endif

extern "C" int oxygen_engine_update_thread(void* data) {

#ifdef OE_PLATFORM_WEB
    while (!oe_threads_ready_to_start)
        SDL_Delay(1);
#endif
    // fetch the task manager and the thread from the pointer
    // execute all functions which are bind to that thread
    oe::thread_data* actual_data = static_cast<oe::thread_data*>(data);

    OE_Main->update_thread(actual_data->thread_id);

    delete actual_data;
    return 0;
}

extern "C" int oxygen_engine_update_unsync_thread(void* data) {

#ifdef OE_PLATFORM_WEB
    while (!oe_threads_ready_to_start)
        SDL_Delay(1);
#endif
    // execute detached threads
    oe::unsync_thread_data* actual_data = static_cast<oe::unsync_thread_data*>(data);

    int output = OE_Main->try_run_unsync_thread(actual_data);

    OE_Main->lockMutex();
    OE_Main->finished_unsync_threadIDs_.insert(actual_data->name);
    OE_Main->unlockMutex();
#ifdef OE_PLATFORM_WEB
    actual_data->~unsync_thread_data();
    ::operator delete(actual_data, std::align_val_t(16));
#else
    delete actual_data;
#endif
    return output;
}


oe::task_manager_t::task_manager_t() {
    completed_threads_ = 0;
    done_              = false;
    started_threads_   = 0;
    countar_           = 0;
    world              = nullptr;
}

bool oe::task_manager_t::is_done() {
    return done_;
}

void oe::task_manager_t::set_pending_world(std::shared_ptr<OE_World> worldin) {
    lockMutex();
    pending_world = worldin;
    unlockMutex();
}
std::shared_ptr<OE_World> oe::task_manager_t::get_world() {
    lockMutex();
    auto output = oe::world;
    unlockMutex();
    return output;
}

void oe::task_manager_t::force_restart_renderer() {
    this->restart_renderer_ = true;
}

/************************
 *  INIT
 * ***********************/

int oe::task_manager_t::init(std::string titlea, int x, int y, bool fullscreen, oe::renderer_init_info renderer_init_info_in,
                             oe::winsys_init_info winsys_init_info_in, oe::physics_init_info physics_init_info_in,
                             oe::networking_init_info networking_init_info_in) {

    this->window_mutex_.lockMutex();
    this->window_             = new oe::sdl_window_system_t();
    this->winsys_init_errors_ = this->try_run_winsys_init(x, y, titlea, fullscreen, winsys_init_info_in);
    this->window_mutex_.unlockMutex();

    this->renderer_mutex_.lockMutex();
    if (winsys_init_info_in.requested_backend == nre::gpu::GLES2)
        this->renderer_ = new nre::renderer_legacy_t();
    else
        this->renderer_ = new nre::renderer_t();

    // do NOT try to initialise the renderer if the window system is borked
    if (not this->winsys_init_errors_)
        this->renderer_init_errors_ = this->try_run_renderer_init(renderer_init_info_in);
    else
        this->renderer_init_errors_ = true;
    this->renderer_mutex_.unlockMutex();

    this->physics_mutex_.lockMutex();
    this->physics_             = new oe::physics_base_t("default");
    this->physics_init_errors_ = this->try_run_physics_init(physics_init_info_in);
    this->physics_mutex_.unlockMutex();

    this->network_             = new oe::networking_base_t("default");
    this->network_init_errors_ = this->try_run_network_init(networking_init_info_in);

    this->create_condition();
    this->create_condition();
    this->create_condition();
    this->create_condition();

    this->set_frame_rate(200);
    this->done_ = false;

    this->create_new_thread("default");
    this->create_new_thread("something else");
#ifdef OE_PLATFORM_WEB

#endif
    if (not this->network_init_errors_)
        oe::create_unsync_thread_method("network", &oe::networking_base_t::execute, this->network_);

#ifdef OE_PLATFORM_WEB
    oe_threads_ready_to_start = true;
#endif
    this->errors_on_init_ =
        this->renderer_init_errors_ or this->physics_init_errors_ or this->winsys_init_errors_ or this->network_init_errors_;
    // cout << "just ran init" << endl;
    return 0;
}

void oe::task_manager_t::create_unsync_thread(string thread_name, const oe::method_type func) {

#ifdef OE_PLATFORM_WEB
    auto threaddata = new (std::align_val_t(16)) oe::unsync_thread_data();
#else
    auto threaddata = new oe::unsync_thread_data();
#endif
    threaddata->func    = func;
    threaddata->name    = thread_name;
    threaddata->taskMgr = this;
    lockMutex();
    this->unsync_threadIDs_[thread_name] =
        SDL_CreateThread(oxygen_engine_update_unsync_thread, thread_name.c_str(), (void*)threaddata);
    unlockMutex();
}

void oe::task_manager_t::create_new_thread(string thread_name) {

    bool synchro = true;

    lockMutex();

    this->threads_.append_now(thread_name, std::shared_ptr<oe::thread_struct>(new oe::thread_struct()));
    size_t thread_id         = this->threads_[thread_name].get_id();
    oe::thread_data::taskMgr = this;
    oe::thread_data* data    = new oe::thread_data();
    data->thread_id          = thread_id;

    if (synchro)
        this->threadIDs_[thread_id] = SDL_CreateThread(oxygen_engine_update_thread, thread_name.c_str(), (void*)data);
    else // USELESS
        this->unsync_threadIDs_[thread_name] = SDL_CreateThread(oxygen_engine_update_thread, thread_name.c_str(), (void*)data);

    unlockMutex();
}

// SYNCHRONIZATION

void oe::task_manager_t::sync_begin_frame() {
    lockMutex();
    started_threads_++;

    if (started_threads_ > get_ready_threads()) {
        started_threads_ = 0;
        cond_broadcast(1);
    }
    else {
        cond_wait(1);
    }
    unlockMutex();
}

void oe::task_manager_t::sync_end_frame() {
    lockMutex();
    completed_threads_++;


    // --------------------------------------------------------------
    // 	Is this really the most elegant and efficient way to solve
    // 	the problem of a zombie thread?		- Anderas, est. 1999
    // --------------------------------------------------------------
    // ------------------ANSWER--------------------------------------
    // No, this is not the place to solve zombie threads. There cannot be
    // any "zombie" threads anyways. Unsynchronized threads are only handled in destroy()
    // This is just for synchronizing all main engine threads each frame.
    // Instead of using ONE mutex condition. The new OE Task Manager
    // uses at minimum 2 and a 3rd one for the physics engine, which would allow you
    // to do all sorts of things with the objects without having to worry about
    // other people accessing them (the synchronization with the renderer happens
    // exactly after this condition on THIS MAIN THREAD in this->renderer->update_data())
    // - Filippos, est. 2056
    // --------------------------------------------------------------

    if (completed_threads_ > get_ready_threads()) {
        /// if this is the last motheyacking thread
        /// that slows down the game, signal all other threads
        /// to continue and reset the counter
        completed_threads_ = 0;
        cond_broadcast(2);
    }
    else {
        /// if this is NOT dat last sh**
        /// wait indefinitely for a signal from the last thread
        cond_wait(2);
    }
    unlockMutex();
}

/************************
 *  STEP
 * ***********************/

void oe::task_manager_t::step() {
    // synchronize at start
    bool temp_done = false;
    if (oe::world != nullptr) {

        // this->physics->world = oe::world;
        // this->renderer->world = oe::world;
        // auto t=clock();
        if (not this->renderer_init_errors_) temp_done = temp_done or this->try_run_renderer_update_data();
        this->restart_renderer_ = false;
        // cout << "NRE UPDATE DATA " << (float)(clock()-t)/CLOCKS_PER_SEC << endl;
    }

    this->update_world();
    // cout << "overcome critical part" << endl;
    this->window_->event_handler_.handle_all_events();

    this->sync_begin_frame();

    if (not this->renderer_init_errors_) temp_done = temp_done or this->try_run_renderer_update_single_thread();

    if (not this->winsys_init_errors_) temp_done = temp_done or this->try_run_winsys_update();
    temp_done = temp_done or this->window_->event_handler_.done();
    // count how many times the step function has been called
    countar_++;

    // THIS is where obsolete unsync threads are cleaned up
    this->remove_finished_unsync_threads();
    done_ = done_ or temp_done;
    this->sync_end_frame();
}


void oe::task_manager_t::start() {
    done_ = false;
    if (this->errors_on_init_) {
        this->destroy();
        return;
    }
    // starts and maintains the game engine
#ifdef OE_PLATFORM_WEB
    emscripten_set_main_loop(&oe::step, 0, true);
#else
    while (!(done_)) {
        this->step();
    }
#endif
    // if an error occurs or the program ends cleanup
    this->destroy();
}


/************************
 *  DESTROY
 * ***********************/

void oe::task_manager_t::destroy() {

    // finish everything
    if (!done_) {
        this->window_->event_handler_.done_ = true;
        this->step();
    }

    lockMutex();
    completed_threads_++;
    unlockMutex();



    if (not this->network_init_errors_) this->network_->destroy();
    int thread_output = 0;
    for (auto thread : this->threadIDs_) {
        SDL_WaitThread(thread.second, &thread_output);
        cout << this->threads_[thread.first]->countar << endl;
    }

    for (auto thread : this->unsync_threadIDs_) {

        SDL_WaitThread(thread.second, &thread_output);
    }
    cout << this->countar_ << endl;

    if (not this->renderer_init_errors_) this->renderer_->destroy();

    if (not this->physics_init_errors_) this->physics_->destroy();

    if (not this->winsys_init_errors_) {
        this->window_->event_handler_.cleanup();
        this->window_->destroy();
    }
    delete this->renderer_;
    delete this->physics_;
    delete this->window_;
    delete this->network_;

    if (oe::world != nullptr) oe::world = nullptr;
    if (oe::pending_world != nullptr) oe::pending_world = nullptr;
}

/************************
 *  THREAD UPDATE
 * ***********************/

void oe::task_manager_t::update_thread(std::size_t thread_id) {

    //  update physics task
    this->threads_[thread_id]->physics_task = oe::task_info_t(0, 0, this->getTicks());
    this->threads_[thread_id]->physics_task.set_type_task(oe::task_type::PHYSICS);

    // obtain the queue in which the tasks are executed if the task queue if changed

    while (!done_) {
        // synchronize at start

        this->sync_begin_frame();
        auto ticks = this->getTicks();

        this->update_task_list(thread_id);
        this->run_thread_tasks(thread_id);

        // for physics engine
        int comp_threads_copy = 0;

        /// synchronize threads to N fps initially
        this->lockMutex();
        unsigned int local_framerate = framerate_;
        unsigned int current_ticks   = this->getTicks() - ticks;
        this->unlockMutex();

        if ((local_framerate) > current_ticks) {
            // this->pause(local_framerate - (current_ticks));
            this->current_framerate_ = 1000.0 / (float)local_framerate;
        }
        else {
            this->current_framerate_ = 1000.0 / (float)(current_ticks);
        }

        // count how many times the thread has been called
        this->threads_[thread_id]->countar++;

        // SYNCHRONIZE PHYSICS
        lockMutex();
        physics_threads_++;
        comp_threads_copy = physics_threads_;
        if (physics_threads_ > (get_ready_threads() - 1)) {
            physics_threads_ = 0;
            this->physics_mutex_.lockMutex();
            if (not this->physics_init_errors_) this->physics_->update_info(this->physics_info_);
            this->physics_mutex_.unlockMutex();
            cond_broadcast(3);
        }
        else {

            cond_wait(3);
        }
        unlockMutex();

        /**************************/
        // This is where physics are run
        this->threads_[thread_id]->physics_task.update();
        if (not this->physics_init_errors_)
            done_ = done_ or this->try_run_physics_update_multi_thread(thread_id, comp_threads_copy);
        /**************************/

        this->sync_end_frame();
    }
}

/************************
 *  OTHER FUNCS
 * ***********************/

// This is for removing finished unsynchronized threads
// Otherwise they will pile up and waste memory on longer gaming sessions
void oe::task_manager_t::remove_finished_unsync_threads() {
    lockMutex();
    for (const auto& x : this->finished_unsync_threadIDs_) {
        SDL_DetachThread(this->unsync_threadIDs_[x]);
        this->unsync_threadIDs_.erase(x);
    }
    this->finished_unsync_threadIDs_.clear();
    unlockMutex();
}

int oe::task_manager_t::get_ready_threads() {
    /// count ALL (i repeat) AAALLLLL threads that are set to be synchronized together
    unsigned int number_of_threads = 0;
    number_of_threads              = this->threads_.size();
    return number_of_threads;
}



void oe::task_manager_t::update_world() {
    lockMutex();

    oe::objects_list.synchronize(oe::pending_world != nullptr);
    oe::materials_list.synchronize(oe::pending_world != nullptr);
    oe::textures_list.synchronize(oe::pending_world != nullptr);
    oe::tcms_list.synchronize(oe::pending_world != nullptr);
    oe::viewports_list.synchronize(oe::pending_world != nullptr);
    oe::scenes_list.synchronize(oe::pending_world != nullptr);


    if (oe::pending_world != nullptr) {
        if (oe::world != nullptr) oe::world = nullptr;
        oe::world = oe::pending_world;
        oe::world->setup();
    }
    oe::pending_world = nullptr;
    unlockMutex();
}

unsigned int oe::task_manager_t::get_frame_rate() {
    // return active framerate
    return this->current_framerate_;
}

void oe::task_manager_t::set_frame_rate(unsigned int frametarget) {
    // set desired framerate
    lockMutex();
    this->framerate_ = 1000u / frametarget;
    unlockMutex();
}

void oe::task_manager_t::add_task(string name, const oe::method_type func) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(0, 0, this->getTicks()));
    task->set_func(func);
    lockMutex();
    this->threads_["default"]->tasks_.force_append(name, task);
    unlockMutex();
}

void oe::task_manager_t::add_task(string name, const oe::method_type func, int priority) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(priority, 0, this->getTicks()));
    task->set_func(func);
    lockMutex();
    this->threads_["default"]->tasks_.force_append(name, task);
    unlockMutex();
}

void oe::task_manager_t::add_task(string name, const oe::method_type func, int priority, string threadname) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(priority, 0, this->getTicks()));
    task->set_func(func);

    lockMutex();

    if (not this->threads_.contains(threadname)) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    this->threads_[threadname]->tasks_.force_append(name, task);

    unlockMutex();
}

void oe::task_manager_t::add_task(string name, const oe::method_type func, string threadname) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(0, 0, this->getTicks()));
    task->set_func(func);
    lockMutex();

    if (not this->threads_.contains(threadname)) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    this->threads_[threadname]->tasks_.force_append(name, task);
    unlockMutex();
}

void oe::task_manager_t::do_once(string name, const oe::method_type func, int delay) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(0, delay, this->getTicks()));
    task->set_func(func);
    lockMutex();
    this->threads_["default"]->tasks_.force_append(name, task);
    unlockMutex();
}

void oe::task_manager_t::remove_task(std::string name) {
    lockMutex();
    size_t task_id = this->threads_["default"]->tasks_.get_id(name);
    this->threads_["default"]->tasks_.remove(task_id);
    unlockMutex();
}

void oe::task_manager_t::remove_task(std::string threadname, std::string name) {
    lockMutex();

    if (not this->threads_.contains(threadname)) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }
    size_t task_id = this->threads_[threadname]->tasks_.get_id(name);
    this->threads_[threadname]->tasks_.remove(task_id);
    unlockMutex();
}

oe::task_info_t oe::task_manager_t::get_task_info(string threadname, string name) {

    oe::task_info_t result;
    lockMutex();

    if (not this->threads_.contains(threadname)) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    auto task_elem = this->threads_[threadname]->tasks_[name];
    if (!task_elem.is_valid()) {
        unlockMutex();
        throw oe::invalid_task_name(name, threadname);
    }

    result = task_elem->get_info();

    unlockMutex();
    return result;
}

void oe::task_manager_t::run_thread_tasks(std::size_t thread_id) {

    std::set<std::size_t> obsolete_tasks;
    for (auto task_elem : this->threads_[thread_id]->tasks_.sorted()) {

        task_elem->update();
        // call the task
        oe::task_action output = this->try_run_task(thread_id, task_elem.get_pointer());
        if (output == oe::task_action::discard) {
            obsolete_tasks.insert(task_elem.get_id());
        }
    }

    for (auto task : obsolete_tasks) {
        this->threads_[thread_id]->tasks_.remove(task);
    }
}

void oe::task_manager_t::update_task_list(std::size_t thread_id) {
    this->threads_[thread_id]->tasks_.synchronize(false);
}

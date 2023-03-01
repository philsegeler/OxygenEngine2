#include <OE/Renderer/renderer_legacy.h>
#include <OE/Renderer/renderer_main.h>
#include <OE/api_oe.h>
#include <OE/math_oe.h>
#include <OE/task_manager.h>
#include <OE/types/libs_oe.h>
#include <OE/winsys_sdl2.h>

using namespace std;
using namespace oe;

std::atomic<std::size_t> OE_ThreadStruct::current_id(0);

OE_ThreadStruct::OE_ThreadStruct() : id(++OE_ThreadStruct::current_id) {
}

oe::task_manager_t* OE_ThreadData::taskMgr = nullptr;

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
    OE_ThreadData* actual_data = static_cast<OE_ThreadData*>(data);

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
    OE_UnsyncThreadData* actual_data = static_cast<OE_UnsyncThreadData*>(data);

    int output = OE_Main->try_run_unsync_thread(actual_data);

    OE_Main->lockMutex();
    OE_Main->finished_unsync_threadIDs.insert(actual_data->name);
    OE_Main->unlockMutex();
#ifdef OE_PLATFORM_WEB
    actual_data->~OE_UnsyncThreadData();
    ::operator delete(actual_data, std::align_val_t(16));
#else
    delete actual_data;
#endif
    return output;
}


oe::task_manager_t::task_manager_t() {
    completed_threads = 0;
    done_             = false;
    started_threads   = 0;
    countar           = 0;
    world             = nullptr;
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
    this->restart_renderer = true;
}

/************************
 *  INIT
 * ***********************/

int oe::task_manager_t::Init(std::string titlea, int x, int y, bool fullscreen, oe::renderer_init_info renderer_init_info_in,
                             oe::winsys_init_info winsys_init_info_in, oe::physics_init_info physics_init_info_in,
                             oe::networking_init_info networking_init_info_in) {

    this->window_mutex.lockMutex();
    this->window             = new oe::sdl_window_system_t();
    this->winsys_init_errors = this->try_run_winsys_init(x, y, titlea, fullscreen, winsys_init_info_in);
    this->window_mutex.unlockMutex();

    this->renderer_mutex.lockMutex();
    if (winsys_init_info_in.requested_backend == nre::gpu::GLES2)
        this->renderer = new NRE_RendererLegacy();
    else
        this->renderer = new NRE_Renderer();

    // do NOT try to initialise the renderer if the window system is borked
    if (not this->winsys_init_errors)
        this->renderer_init_errors = this->try_run_renderer_init(renderer_init_info_in);
    else
        this->renderer_init_errors = true;
    this->renderer_mutex.unlockMutex();

    this->physics_mutex.lockMutex();
    this->physics             = new oe::physics_base_t("default");
    this->physics_init_errors = this->try_run_physics_init(physics_init_info_in);
    this->physics_mutex.unlockMutex();

    this->network             = new oe::networking_base_t("default");
    this->network_init_errors = this->try_run_network_init(networking_init_info_in);

    this->createCondition();
    this->createCondition();
    this->createCondition();
    this->createCondition();

    this->set_frame_rate(200);
    this->done_ = false;

    this->CreateNewThread("default");
    this->CreateNewThread("something else");
#ifdef OE_PLATFORM_WEB

#endif
    if (not this->network_init_errors)
        oe::create_unsync_thread_method("network", &oe::networking_base_t::execute, this->network);

#ifdef OE_PLATFORM_WEB
    oe_threads_ready_to_start = true;
#endif
    this->errors_on_init =
        this->renderer_init_errors or this->physics_init_errors or this->winsys_init_errors or this->network_init_errors;
    // cout << "just ran init" << endl;
    return 0;
}

void oe::task_manager_t::CreateUnsyncThread(string thread_name, const oe::method_type func) {

#ifdef OE_PLATFORM_WEB
    auto threaddata = new (std::align_val_t(16)) OE_UnsyncThreadData();
#else
    auto threaddata = new OE_UnsyncThreadData();
#endif
    threaddata->func    = func;
    threaddata->name    = thread_name;
    threaddata->taskMgr = this;
    lockMutex();
    this->unsync_threadIDs[thread_name] =
        SDL_CreateThread(oxygen_engine_update_unsync_thread, thread_name.c_str(), (void*)threaddata);
    unlockMutex();
}

void oe::task_manager_t::CreateNewThread(string thread_name) {

    bool synchro = true;

    lockMutex();

    this->threads.append_now(thread_name, std::shared_ptr<OE_ThreadStruct>(new OE_ThreadStruct()));
    size_t thread_id       = this->threads[thread_name].get_id();
    OE_ThreadData::taskMgr = this;
    OE_ThreadData* data    = new OE_ThreadData();
    data->thread_id        = thread_id;

    if (synchro)
        this->threadIDs[thread_id] = SDL_CreateThread(oxygen_engine_update_thread, thread_name.c_str(), (void*)data);
    else // USELESS
        this->unsync_threadIDs[thread_name] = SDL_CreateThread(oxygen_engine_update_thread, thread_name.c_str(), (void*)data);

    unlockMutex();
}

// SYNCHRONIZATION

void oe::task_manager_t::sync_begin_frame() {
    lockMutex();
    started_threads++;

    if (started_threads > get_ready_threads()) {
        started_threads = 0;
        condBroadcast(1);
    }
    else {
        condWait(1);
    }
    unlockMutex();
}

void oe::task_manager_t::sync_end_frame() {
    lockMutex();
    completed_threads++;


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

    if (completed_threads > get_ready_threads()) {
        /// if this is the last motheyacking thread
        /// that slows down the game, signal all other threads
        /// to continue and reset the counter
        completed_threads = 0;
        condBroadcast(2);
    }
    else {
        /// if this is NOT dat last sh**
        /// wait indefinitely for a signal from the last thread
        condWait(2);
    }
    unlockMutex();
}

/************************
 *  STEP
 * ***********************/

void oe::task_manager_t::Step() {
    // synchronize at start
    bool temp_done = false;
    if (oe::world != nullptr) {

        // this->physics->world = oe::world;
        // this->renderer->world = oe::world;
        // auto t=clock();
        if (not this->renderer_init_errors) temp_done = temp_done or this->try_run_renderer_update_data();
        this->restart_renderer = false;
        // cout << "NRE UPDATE DATA " << (float)(clock()-t)/CLOCKS_PER_SEC << endl;
    }

    this->update_world();
    // cout << "overcome critical part" << endl;
    this->window->event_handler_.handle_all_events();

    this->sync_begin_frame();

    if (not this->renderer_init_errors) temp_done = temp_done or this->try_run_renderer_update_single_thread();

    if (not this->winsys_init_errors) temp_done = temp_done or this->try_run_winsys_update();
    temp_done = temp_done or this->window->event_handler_.done();
    // count how many times the step function has been called
    countar++;

    // THIS is where obsolete unsync threads are cleaned up
    this->remove_finished_unsync_threads();
    done_ = done_ or temp_done;
    this->sync_end_frame();
}


void oe::task_manager_t::Start() {
    done_ = false;
    if (this->errors_on_init) {
        this->Destroy();
        return;
    }
    // starts and maintains the game engine
#ifdef OE_PLATFORM_WEB
    emscripten_set_main_loop(&oe::step, 0, true);
#else
    while (!(done_)) {
        this->Step();
    }
#endif
    // if an error occurs or the program ends cleanup
    this->Destroy();
}


/************************
 *  DESTROY
 * ***********************/

void oe::task_manager_t::Destroy() {

    // finish everything
    if (!done_) {
        this->window->event_handler_.done_ = true;
        this->Step();
    }

    lockMutex();
    completed_threads++;
    unlockMutex();



    if (not this->network_init_errors) this->network->destroy();
    int thread_output = 0;
    for (auto thread : this->threadIDs) {
        SDL_WaitThread(thread.second, &thread_output);
        cout << this->threads[thread.first]->countar << endl;
    }

    for (auto thread : this->unsync_threadIDs) {

        SDL_WaitThread(thread.second, &thread_output);
    }
    cout << this->countar << endl;

    if (not this->renderer_init_errors) this->renderer->destroy();

    if (not this->physics_init_errors) this->physics->destroy();

    if (not this->winsys_init_errors) {
        this->window->event_handler_.cleanup();
        this->window->destroy();
    }
    delete this->renderer;
    delete this->physics;
    delete this->window;
    delete this->network;

    if (oe::world != nullptr) oe::world = nullptr;
    if (oe::pending_world != nullptr) oe::pending_world = nullptr;

    this->destroy(); // from OE_MutexCondition
}

/************************
 *  THREAD UPDATE
 * ***********************/

void oe::task_manager_t::update_thread(std::size_t thread_id) {

    //  update physics task
    this->threads[thread_id]->physics_task = oe::task_info_t(0, 0, this->getTicks());
    this->threads[thread_id]->physics_task.set_type_task(oe::task_type::PHYSICS);

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
        unsigned int local_framerate = framerate;
        unsigned int current_ticks   = this->getTicks() - ticks;
        this->unlockMutex();

        if ((local_framerate) > current_ticks) {
            // this->pause(local_framerate - (current_ticks));
            this->current_framerate = 1000.0 / (float)local_framerate;
        }
        else {
            this->current_framerate = 1000.0 / (float)(current_ticks);
        }

        // count how many times the thread has been called
        this->threads[thread_id]->countar++;

        // SYNCHRONIZE PHYSICS
        lockMutex();
        physics_threads++;
        comp_threads_copy = physics_threads;
        if (physics_threads > (get_ready_threads() - 1)) {
            physics_threads = 0;
            this->physics_mutex.lockMutex();
            if (not this->physics_init_errors) this->physics->update_info(this->physics_info);
            this->physics_mutex.unlockMutex();
            condBroadcast(3);
        }
        else {

            condWait(3);
        }
        unlockMutex();

        /**************************/
        // This is where physics are run
        this->threads[thread_id]->physics_task.update();
        if (not this->physics_init_errors)
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
    for (const auto& x : this->finished_unsync_threadIDs) {
        SDL_DetachThread(this->unsync_threadIDs[x]);
        this->unsync_threadIDs.erase(x);
    }
    this->finished_unsync_threadIDs.clear();
    unlockMutex();
}

int oe::task_manager_t::get_ready_threads() {
    /// count ALL (i repeat) AAALLLLL threads that are set to be synchronized together
    unsigned int number_of_threads = 0;
    number_of_threads              = this->threads.size();
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
    return this->current_framerate;
}

void oe::task_manager_t::set_frame_rate(unsigned int frametarget) {
    // set desired framerate
    lockMutex();
    this->framerate = 1000u / frametarget;
    unlockMutex();
}

void oe::task_manager_t::AddTask(string name, const oe::method_type func) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(0, 0, this->getTicks()));
    task->set_func(func);
    lockMutex();
    this->threads["default"]->tasks_.force_append(name, task);
    unlockMutex();
}

void oe::task_manager_t::AddTask(string name, const oe::method_type func, int priority) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(priority, 0, this->getTicks()));
    task->set_func(func);
    lockMutex();
    this->threads["default"]->tasks_.force_append(name, task);
    unlockMutex();
}

void oe::task_manager_t::AddTask(string name, const oe::method_type func, int priority, string threadname) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(priority, 0, this->getTicks()));
    task->set_func(func);

    lockMutex();

    if (not this->threads.contains(threadname)) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    this->threads[threadname]->tasks_.force_append(name, task);

    unlockMutex();
}

void oe::task_manager_t::AddTask(string name, const oe::method_type func, string threadname) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(0, 0, this->getTicks()));
    task->set_func(func);
    lockMutex();

    if (not this->threads.contains(threadname)) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    this->threads[threadname]->tasks_.force_append(name, task);
    unlockMutex();
}

void oe::task_manager_t::DoOnce(string name, const oe::method_type func, int delay) {

    std::shared_ptr<oe::task_t> task = std::shared_ptr<oe::task_t>(new oe::task_t(0, delay, this->getTicks()));
    task->set_func(func);
    lockMutex();
    this->threads["default"]->tasks_.force_append(name, task);
    unlockMutex();
}

void oe::task_manager_t::RemoveTask(std::string name) {
    lockMutex();
    size_t task_id = this->threads["default"]->tasks_.get_id(name);
    this->threads["default"]->tasks_.remove(task_id);
    unlockMutex();
}

void oe::task_manager_t::RemoveTask(std::string threadname, std::string name) {
    lockMutex();

    if (not this->threads.contains(threadname)) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }
    size_t task_id = this->threads[threadname]->tasks_.get_id(name);
    this->threads[threadname]->tasks_.remove(task_id);
    unlockMutex();
}

oe::task_info_t oe::task_manager_t::get_task_info(string threadname, string name) {

    oe::task_info_t result;
    lockMutex();

    if (not this->threads.contains(threadname)) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    auto task_elem = this->threads[threadname]->tasks_[name];
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
    for (auto task_elem : this->threads[thread_id]->tasks_.sorted()) {

        task_elem->update();
        // call the task
        oe::task_action output = this->try_run_task(thread_id, task_elem.get_pointer());
        if (output == oe::task_action::discard) {
            obsolete_tasks.insert(task_elem.get_id());
        }
    }

    for (auto task : obsolete_tasks) {
        this->threads[thread_id]->tasks_.remove(task);
    }
}

void oe::task_manager_t::update_task_list(std::size_t thread_id) {
    this->threads[thread_id]->tasks_.synchronize(false);
}

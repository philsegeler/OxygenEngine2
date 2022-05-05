#include <OE/Renderer/renderer_legacy.h>
#include <OE/Renderer/renderer_main.h>
#include <OE/api_oe.h>
#include <OE/math_oe.h>
#include <OE/task_manager.h>
#include <OE/types/libs_oe.h>
#include <OE/winsys_sdl2.h>

using namespace std;
using namespace oe;

OE_TaskManager* OE_ThreadData::taskMgr = nullptr;

OE_ThreadStruct::OE_ThreadStruct() {
}

OE_ThreadStruct::~OE_ThreadStruct() {
}

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

    // cout << "this is actual data " << actual_data << " " << OE_Main << endl;
    //  update physics task
    OE_Main->threads[actual_data->name].physics_task =
        OE_Task(actual_data->name + "-physics", 0, 0, OE_ThreadData::taskMgr->getTicks());

    OE_Main->updateThread(actual_data->name);

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

    int output = OE_Main->tryRun_unsync_thread(actual_data);

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

void OE_ThreadStruct::updateTaskList() {

    // erase all unneeded tasks in the beginning
    for (size_t x = 0; x < this->to_be_removed.size(); x++) {
        for (size_t i = 0; i < this->tasks.size(); i++) {
            if (this->tasks[i].GetName() == this->to_be_removed.front()) {
                this->tasks.erase(this->tasks.begin() + i);
                this->functions.erase(this->tasks[i].name);
            }
        }
        this->to_be_removed.pop();
    }

    // add all tasks of the previous frame
    for (auto x : std::exchange(this->pending_tasks, {})) {
        if (this->task_names.count(x.name) == 1) {
            cout << "[OE Warning] Task Manager: Already existing task: " << x.name << endl;
            OE_WriteToLog("[OE Warning] Task Manager: Already existing task: " + x.name);

            // reset previous task
            for (size_t i = 0; i < this->tasks.size(); i++) {
                if (this->tasks[i].name == x.name) {
                    this->tasks[i]          = x;
                    this->functions[x.name] = this->pending_functions[x.name];
                    break;
                }
            }

            continue;
        }
        this->tasks.push_back(x);
        this->task_names.insert(x.name);
        this->functions[x.name] = this->pending_functions[x.name];
    }
    this->pending_functions.clear();
}



OE_TaskManager::OE_TaskManager() {
    completed_threads = 0;
    done              = false;
    started_threads   = 0;
    countar           = 0;
    world             = nullptr;
}

OE_TaskManager::~OE_TaskManager() {
}

/************************
 *  INIT
 * ***********************/

int OE_TaskManager::Init(std::string titlea, int x, int y, bool fullscreen, oe::renderer_init_info renderer_init_info_in,
                         oe::winsys_init_info winsys_init_info_in, oe::physics_init_info physics_init_info_in,
                         oe::networking_init_info networking_init_info_in) {

    this->window_mutex.lockMutex();
    this->window             = new OE_SDL_WindowSystem();
    this->winsys_init_errors = this->tryRun_winsys_init(x, y, titlea, fullscreen, winsys_init_info_in);
    this->window_mutex.unlockMutex();

    this->renderer_mutex.lockMutex();
    if (winsys_init_info_in.requested_backend == nre::gpu::GLES2)
        this->renderer = new NRE_RendererLegacy();
    else
        this->renderer = new NRE_Renderer();

    // do NOT try to initialise the renderer if the window system is borked
    if (not this->winsys_init_errors)
        this->renderer_init_errors = this->tryRun_renderer_init(renderer_init_info_in);
    else
        this->renderer_init_errors = true;
    this->renderer_mutex.unlockMutex();

    this->physics_mutex.lockMutex();
    this->physics             = new oe::physics_base_t();
    this->physics_init_errors = this->tryRun_physics_init(physics_init_info_in);
    this->physics_mutex.unlockMutex();

    this->network             = new oe::networking_base_t();
    this->network_init_errors = this->tryRun_network_init(networking_init_info_in);

    this->createCondition();
    this->createCondition();
    this->createCondition();
    this->createCondition();

    this->SetFrameRate(200);
    this->done = false;

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

void OE_TaskManager::CreateUnsyncThread(string thread_name, const OE_METHOD func) {

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

void OE_TaskManager::CreateNewThread(string thread_name) {

    bool synchro = true;

    lockMutex();

    this->threads[thread_name]             = OE_ThreadStruct();
    this->threads[thread_name].synchronize = true;
    this->threads[thread_name].changed     = true;

    OE_ThreadData::taskMgr = this;
    OE_ThreadData* data    = new OE_ThreadData();
    data->name             = thread_name;

    if (synchro)
        this->threadIDs[thread_name] = SDL_CreateThread(oxygen_engine_update_thread, thread_name.c_str(), (void*)data);
    else // USELESS
        this->unsync_threadIDs[thread_name] = SDL_CreateThread(oxygen_engine_update_thread, thread_name.c_str(), (void*)data);

    unlockMutex();
}

// SYNCHRONIZATION

void OE_TaskManager::syncBeginFrame() {
    lockMutex();
    started_threads++;

    if (started_threads > getReadyThreads()) {
        started_threads = 0;
        condBroadcast(1);
    }
    else {
        condWait(1);
    }
    unlockMutex();
}

void OE_TaskManager::syncEndFrame() {
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
    // exactly after this condition on THIS MAIN THREAD in this->renderer->updateData())
    // - Filippos, est. 2056
    // --------------------------------------------------------------

    if (completed_threads > getReadyThreads()) {
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

void OE_TaskManager::Step() {
    // synchronize at start
    bool temp_done = false;
    if (this->world != nullptr) {

        // this->physics->world = this->world;
        // this->renderer->world = this->world;
        // auto t=clock();
        if (not this->renderer_init_errors) temp_done = temp_done or this->tryRun_renderer_updateData();
        this->restart_renderer = false;
        // cout << "NRE UPDATE DATA " << (float)(clock()-t)/CLOCKS_PER_SEC << endl;
    }

    this->updateWorld();
    // cout << "overcome critical part" << endl;
    this->window->event_handler_.handle_all_events();

    this->syncBeginFrame();

    if (not this->renderer_init_errors) temp_done = temp_done or this->tryRun_renderer_updateSingleThread();

    if (not this->winsys_init_errors) temp_done = temp_done or this->tryRun_winsys_update();
    temp_done = temp_done or this->window->event_handler_.done();
    // count how many times the step function has been called
    countar++;

    // THIS is where obsolete unsync threads are cleaned up
    this->removeFinishedUnsyncThreads();
    done = done or temp_done;
    this->syncEndFrame();
}


void OE_TaskManager::Start() {
    done = false;
    if (this->errors_on_init) {
        this->Destroy();
        return;
    }
    // starts and maintains the game engine
#ifdef OE_PLATFORM_WEB
    emscripten_set_main_loop(&oe::step, 0, true);
#else
    while (!(done)) {
        this->Step();
    }
#endif
    // if an error occurs or the program ends cleanup
    this->Destroy();
}


/************************
 *  DESTROY
 * ***********************/

void OE_TaskManager::Destroy() {

    // finish everything
    if (!done) {
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
        cout << this->threads[thread.first].countar << endl;
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

    if (this->world != nullptr) this->world = nullptr;
    if (this->pending_world != nullptr) this->pending_world = nullptr;

    this->destroy(); // from OE_MutexCondition
}

/************************
 *  THREAD UPDATE
 * ***********************/

void OE_TaskManager::updateThread(const string name) {
    // obtain the queue in which the tasks are executed if the task queue if changed

    while (!done) {
        // synchronize at start
        this->syncBeginFrame();
        auto ticks = this->getTicks();
        if (this->threads[name].changed) {
            this->sortThreadTasks(name);
        }
        this->runThreadTasks(name);

        // this if statement should normally be redundant but it crashes without it. No idea why
        if (this->threads[name].synchronize == true) {

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
            this->threads[name].countar++;

            // SYNCHRONIZE PHYSICS
            lockMutex();
            physics_threads++;
            comp_threads_copy = physics_threads;
            if (physics_threads > (getReadyThreads() - 1)) {
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
            this->threads[name].physics_task.update();
            if (not this->physics_init_errors) done = done or this->tryRun_physics_updateMultiThread(name, comp_threads_copy);
            /**************************/

            this->syncEndFrame();
        }
    }
}

/************************
 *  OTHER FUNCS
 * ***********************/

// This is for removing finished unsynchronized threads
// Otherwise they will pile up and waste memory on longer gaming sessions
void OE_TaskManager::removeFinishedUnsyncThreads() {
    lockMutex();
    for (const auto& x : this->finished_unsync_threadIDs) {
        SDL_DetachThread(this->unsync_threadIDs[x]);
        this->unsync_threadIDs.erase(x);
    }
    this->finished_unsync_threadIDs.clear();
    unlockMutex();
}

int OE_TaskManager::getReadyThreads() {
    /// count ALL (i repeat) AAALLLLL threads that are set to be synchronized together
    unsigned int number_of_threads = 0;
    // WARNING: For some reason this causes a segmentation fault sometimes and i have no idea why
    /*for(auto thread: threads){
        if(thread.second.synchronize){
            number_of_threads+=1;
        }
    }*/
    number_of_threads = this->threads.size();
    return number_of_threads;
}



void OE_TaskManager::updateWorld() {
    lockMutex();

    OE_World::objectsList.synchronize(this->pending_world != nullptr);
    OE_World::materialsList.synchronize(this->pending_world != nullptr);
    OE_World::texturesList.synchronize(this->pending_world != nullptr);
    OE_World::tcmsList.synchronize(this->pending_world != nullptr);
    OE_World::viewportsList.synchronize(this->pending_world != nullptr);
    OE_World::scenesList.synchronize(this->pending_world != nullptr);


    if (this->pending_world != nullptr) {
        if (this->world != nullptr) this->world = nullptr;
        this->world = this->pending_world;
        this->world->setup();
    }
    this->pending_world = nullptr;

    this->physics->world  = this->world;
    this->renderer->world = this->world;
    unlockMutex();
}

unsigned int OE_TaskManager::GetFrameRate() {
    // return active framerate
    return this->current_framerate;
}

void OE_TaskManager::SetFrameRate(unsigned int frametarget) {
    // set desired framerate
    lockMutex();
    this->framerate = 1000u / frametarget;
    unlockMutex();
}

void OE_TaskManager::AddTask(string name, const OE_METHOD func) {

    OE_Task task = OE_Task(name, 0, 0, this->getTicks());
    lockMutex();

    this->threads["default"].pending_tasks.push_back(task);
    this->threads["default"].pending_functions[name] = func;
    this->threads["default"].changed                 = true;
    unlockMutex();
}

void OE_TaskManager::AddTask(string name, const OE_METHOD func, int priority) {

    OE_Task task = OE_Task(name, priority, 0, this->getTicks());
    lockMutex();
    this->threads["default"].pending_tasks.push_back(task);
    this->threads["default"].pending_functions[name] = func;
    this->threads["default"].changed                 = true;
    unlockMutex();
}

void OE_TaskManager::AddTask(string name, const OE_METHOD func, int priority, string threadname) {

    OE_Task task = OE_Task(name, priority, 0, this->getTicks());

    lockMutex();

    if (this->threads.count(threadname) < 1) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    this->threads[threadname].pending_tasks.push_back(task);
    this->threads[threadname].pending_functions[name] = func;
    this->threads[threadname].changed                 = true;
    unlockMutex();
}

void OE_TaskManager::AddTask(string name, const OE_METHOD func, string threadname) {

    OE_Task task = OE_Task(name, 0, 0, this->getTicks());
    lockMutex();

    if (this->threads.count(threadname) < 1) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    this->threads[threadname].pending_tasks.push_back(task);
    this->threads[threadname].pending_functions[name] = func;
    this->threads[threadname].changed                 = true;
    unlockMutex();
}

void OE_TaskManager::DoOnce(string name, const OE_METHOD func, int delay) {

    OE_Task task = OE_Task(name, 0, delay, this->getTicks());
    lockMutex();
    this->threads["default"].pending_tasks.push_back(task);
    this->threads["default"].pending_functions[name] = func;
    this->threads["default"].changed                 = true;
    unlockMutex();
}

void OE_TaskManager::RemoveTask(std::string name) {
    lockMutex();
    this->threads["default"].to_be_removed.push(name);
    unlockMutex();
}

void OE_TaskManager::RemoveTask(std::string threadname, std::string name) {
    lockMutex();

    if (this->threads.count(threadname) < 1) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    this->threads[threadname].to_be_removed.push(name);
    unlockMutex();
}

OE_Task OE_TaskManager::GetTaskInfo(string threadname, string name) {

    OE_Task result;
    lockMutex();

    if (this->threads.count(threadname) < 1) {
        unlockMutex();
        throw oe::invalid_thread_name(threadname);
    }

    if (this->threads[threadname].task_names.count(name) < 1) {
        unlockMutex();
        throw oe::invalid_task_name(name, threadname);
    }

    for (auto task : this->threads[threadname].tasks) {
        if (task.GetName() == name) {
            result = task;
            break;
        }
    }

    unlockMutex();
    return result;
}

void OE_TaskManager::runThreadTasks(const std::string& name) {
    unsigned int tasks_size = this->threads[name].functions.size();

    // after updating the task queue start executing the functions
    if (tasks_size != 0) {

        std::set<OE_Task, std::greater<OE_Task>> obsolete_tasks;

        unsigned int task_index = 0;
        for (auto&& task : this->threads[name].tasks) {

            // store active task
            this->active_tasks[name] = task.GetName();
            task.update();
            // call the task
            int output = 0;
            output     = this->tryRun_task(name, task);
            switch (output) {
            case 1:
                obsolete_tasks.insert(task);
                break;
            }

            task_index++;
        }



        for (auto task : obsolete_tasks) {
            this->threads[name].functions.erase(task.name);
            auto task_id = std::find(this->threads[name].tasks.begin(), this->threads[name].tasks.end(), task);
            this->threads[name].tasks.erase(task_id);
        }
        obsolete_tasks.clear();
    }
}

void OE_TaskManager::sortThreadTasks(const std::string& name) {

    lockMutex();
    this->threads[name].updateTaskList();
    unlockMutex();

    std::sort(this->threads[name].tasks.begin(), this->threads[name].tasks.end(), std::greater<OE_Task>());
}

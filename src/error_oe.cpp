#include <OE/Events/event.h>
#include <OE/error_oe.h>
#include <OE/task_manager.h>
#include <OE/types/libs_oe.h>

using namespace std;

// This is where events' error handling is happening
int oe::event_handler_t::call_event(size_t event_id) {

    /// generic event management
    auto event = events_list_[event_id];
    if (event.is_valid()) {

        event->lockMutex();
        try {
            event->call();
        } catch (oe::api_error& e) {
            std::string error_str = "[OE Error] '" + e.name_ + "' thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.GetCounter()) + "\n";
            error_str += "\t" + e.what() + "\n";
            cout << error_str;
            OE_WriteToLog(error_str);
        } catch (csl::parser_error_t& e) {
            std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.GetCounter()) + "\n";
            error_str += "\t" + e.what() + "\n";
            cout << error_str;
            OE_WriteToLog(error_str);
        } catch (csl::interpreter_error_t& e) {
            std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.GetCounter()) + "\n";
            error_str += "\t" + e.what() + "\n";
            cout << error_str;
            OE_WriteToLog(error_str);
        } catch (std::exception& e) {
            std::string error_str = "[OE Error] std::exception variant thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.GetCounter()) + "\n";
            error_str += "\t" + string(e.what()) + "\n";
            cout << error_str;
            OE_WriteToLog(error_str);
        } catch (...) {
            std::string error_str = "[OE Error] Exception thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.GetCounter());
            cout << error_str << endl;
            OE_WriteToLog(error_str + "\n");
        }
        event->unlockMutex();
    }
    else {
        // TODO: Warning
    }

    return 0;
}

// error handling functions
int OE_TaskManager::tryRun_unsync_thread(OE_UnsyncThreadData* actual_data) {

    int output = 1;

    OE_Task unsync_task = OE_Task(actual_data->name, 0, 0, actual_data->taskMgr->getTicks());
    try {
        output = actual_data->func(unsync_task);
    } catch (oe::api_error& e) {
        std::string error_str = "[OE Error] '" + e.name_ + "' thrown in unsync thread: '" + unsync_task.GetName() + "'" + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (csl::parser_error_t& e) {
        std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in unsync thread: '" + unsync_task.GetName() + "'" + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (csl::interpreter_error_t& e) {
        std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in unsync thread: '" + unsync_task.GetName() + "'" + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (oe::networking_error& e) {
        std::string error_str = "[SLC Error] '" + e.name_ + "' thrown in unsync thread: '" + unsync_task.GetName() + "'" + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (std::exception& e) {
        std::string error_str =
            "[OE Error] std::exception variant thrown in unsync thread: '" + unsync_task.GetName() + "'" + "\n";
        error_str += "\t" + string(e.what()) + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (...) {
        std::string error_str = "[OE Error] Exception thrown in unsync thread: '" + unsync_task.GetName() + "'";
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return output;
}

int OE_TaskManager::tryRun_task(const std::string& name, OE_Task& task) {

    int output = 0;

    try {
        if (this->threads[name].functions[task.name] != nullptr) output = this->threads[name].functions[task.name](task);
    } catch (oe::api_error& e) {
        std::string error_str = "[OE Error] '" + e.name_ + "' thrown in task: '" + task.name + "', thread: '" + name;
        error_str += "', invocation: " + std::to_string(task.counter) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
        output = 1;
    } catch (csl::parser_error_t& e) {
        std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in task: '" + task.name + "', thread: '" + name;
        error_str += "', invocation: " + std::to_string(task.counter) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
        output = 1;
    } catch (csl::interpreter_error_t& e) {
        std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in task: '" + task.name + "', thread: '" + name;
        error_str += "', invocation: " + std::to_string(task.counter) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
        output = 1;
    } catch (std::exception& e) {
        std::string error_str =
            "[OE Error] '" + string(typeid(e).name()) + "' thrown in task: '" + task.name + "', thread: '" + name;
        error_str += "', invocation: " + std::to_string(task.counter) + "\n";
        error_str += "\t" + string(e.what()) + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
        output = 1;
    } catch (...) {
        /// universal error handling. will catch any exception
        /// feel free to add specific handling for specific errors
        string outputa = string("[OE Error] Exception thrown in task: '" + task.name + "', thread: '" + name);
        outputa += "', invocation: " + std::to_string(task.counter);
        cout << outputa << endl;
        OE_WriteToLog(outputa + "\n");
        output = 1;
    }
    return output;
}

bool OE_TaskManager::tryRun_physics_updateMultiThread(const std::string& name, const int& comp_threads_copy) {


    try {
        this->physics->update_multi_thread(&this->threads[name].physics_task, comp_threads_copy);
        return false;
    } catch (oe::api_error& e) {
        std::string error_str = "[HPE Error] '" + e.name_ + "' thrown in update_multi_thread of '" + this->physics->get_name() +
                                "' in  thread: '" + name + "', thread_num: " + std::to_string(comp_threads_copy);
        error_str += ", invocation: " + std::to_string(this->threads[name].physics_task.counter) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (oe::physics_error& e) {
        std::string error_str = "[HPE Error] '" + e.name_ + "' thrown in update_multi_thread of '" + this->physics->get_name() +
                                "' in  thread: '" + name + "', thread_num: " + std::to_string(comp_threads_copy);
        error_str += ", invocation: " + std::to_string(this->threads[name].physics_task.counter) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (std::exception& e) {
        std::string error_str = "[HPE Error] '" + string(typeid(e).name()) + "' thrown in update_multi_thread of '" +
                                this->physics->get_name() + "' in  thread: '" + name +
                                "', thread_num: " + std::to_string(comp_threads_copy);
        error_str += ", invocation: " + std::to_string(this->threads[name].physics_task.counter) + "\n";
        error_str += "\t" + string(e.what()) + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (...) {
        /// universal error handling. will catch any exception
        /// feel free to add specific handling for specific errors
        auto   task    = this->threads[name].physics_task;
        string outputa = string("[HPE Error] Physics exception thrown in update_multi_thread of '" + this->physics->get_name() +
                                "' in  thread: '" + name + "', thread_num: " + std::to_string(comp_threads_copy));
        outputa += ", invocation: " + std::to_string(task.counter);
        cout << outputa << endl;
        OE_WriteToLog(outputa + "\n");
    }
    return true;
}

bool OE_TaskManager::tryRun_renderer_updateSingleThread() {

    this->renderer_mutex.lockMutex();
    auto renderer_info_copy = this->renderer_info;
    this->renderer_mutex.unlockMutex();

    try {
        this->renderer->update_single_thread(renderer_info_copy, this->window_output);
        return false;
    } catch (oe::api_error& e) {
        std::string error_str = "[NRE Error] '" + e.name_ + "' thrown in update_single_thread of '" +
                                this->renderer->get_name() + "', invocation: " + std::to_string(this->countar);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (oe::renderer_error& e) {
        std::string error_str = "[NRE Error] '" + e.name_ + "' thrown in update_single_thread of '" +
                                this->renderer->get_name() + "', invocation: " + std::to_string(this->countar);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[NRE Error] '" + string(typeid(e).name()) + "' thrown in update_single_thread of '" +
                                this->renderer->get_name() + "', invocation: " + std::to_string(this->countar);
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[NRE Error] Renderer exception thrown in update_single_thread of '" +
                                this->renderer->get_name() + "', invocation: " + std::to_string(this->countar);
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

bool OE_TaskManager::tryRun_renderer_updateData() {

    this->renderer_mutex.lockMutex();
    auto renderer_info_copy = this->renderer_info;
    this->renderer_mutex.unlockMutex();

    try {
        this->renderer->update_data(renderer_info_copy, this->window_output, this->restart_renderer);
        return false;
    } catch (oe::api_error& e) {
        std::string error_str = "[NRE Error] '" + e.name_ + "' thrown in update_data of '" + this->renderer->get_name() +
                                "', invocation: " + std::to_string(this->countar);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (oe::renderer_error& e) {
        std::string error_str = "[NRE Error] '" + e.name_ + "' thrown in update_data of '" + this->renderer->get_name() +
                                "', invocation: " + std::to_string(this->countar);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[NRE Error] '" + string(typeid(e).name()) + "' thrown in update_data of '" +
                                this->renderer->get_name() + "', invocation: " + std::to_string(this->countar);
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[NRE Error] Renderer exception thrown in update_data of '" + this->renderer->get_name() +
                                "', invocation: " + std::to_string(this->countar);
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

bool OE_TaskManager::tryRun_winsys_update() {

    this->window_mutex.lockMutex();
    auto winsys_info = this->window_info;
    this->window_mutex.unlockMutex();

    try {
        this->window_output = this->window->update(winsys_info);
        return this->window_output.done;
    } catch (oe::winsys_error& e) {
        std::string error_str =
            "[OE WINSYS Error] '" + e.name_ + "' thrown in update, invocation: " + std::to_string(this->countar);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[OE WINSYS Error] '" + string(typeid(e).name()) +
                                "' thrown in update, invocation: " + std::to_string(this->countar);
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[OE WINSYS Error] Exception thrown in update, invocation: " + std::to_string(this->countar);
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }

    return true;
}

bool OE_TaskManager::tryRun_winsys_init(int x, int y, std::string titlea, bool fullscreen, oe::winsys_init_info params) {

    this->window_init_info           = params;
    this->window_info.res_x          = x;
    this->window_info.res_y          = y;
    this->window_info.use_fullscreen = fullscreen;
    this->window_info.title          = titlea;

    try {
        this->window_output = this->window->init(this->window_init_info, this->window_info);
        return false;
    } catch (oe::winsys_error& e) {
        std::string error_str = "[OE WINSYS Error] '" + e.name_ + "' thrown in window system initialization";
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[OE WINSYS Error] '" + string(typeid(e).name()) + "' thrown in window system initialization";
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str =
            "[OE WINSYS Error] Could not initialize window system due to thrown exception in window->init()";
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

bool OE_TaskManager::tryRun_physics_init(oe::physics_init_info params) {

    try {
        this->physics_init_info = params;
        this->physics->init(params);
        return false;
    } catch (oe::physics_error& e) {
        std::string error_str =
            "[HPE Error] '" + e.name_ + "' thrown in physics engine '" + this->physics->get_name() + "' initialization";
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[HPE Error] '" + string(typeid(e).name()) + "' thrown in physics engine '" +
                                this->physics->get_name() + "' initialization";
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[HPE Error] Could not initialize physics engine '" + this->physics->get_name() +
                                "' due to thrown exception in physics->init()";
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

bool OE_TaskManager::tryRun_renderer_init(oe::renderer_init_info params) {

    try {
        this->renderer_init_info = params;
        this->renderer->init(this->renderer_init_info, this->renderer_info, this->window_output);
        return false;
    } catch (oe::renderer_error& e) {
        std::string error_str =
            "[NRE Error] '" + e.name_ + "' thrown in renderer '" + this->renderer->get_name() + "' initialization";
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[NRE Error] '" + string(typeid(e).name()) + "' thrown in renderer '" +
                                this->renderer->get_name() + "' initialization";
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[NRE Error] Could not initialize renderer due to thrown exception in renderer '" +
                                this->renderer->get_name() + "' initialization";
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

bool OE_TaskManager::tryRun_network_init(oe::networking_init_info params) {
    try {
        this->network_init_info = params;
        this->network->init(params);
        return false;
    } catch (oe::networking_error& e) {
        std::string error_str =
            "[SLC Error] '" + e.name_ + "' thrown in networking '" + this->network->get_name() + "' initialization";
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[SLC Error] '" + string(typeid(e).name()) + "' thrown in networking '" +
                                this->network->get_name() + "' initialization";
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[SLC Error] Could not initialize networking due to thrown exception in networking '" +
                                this->network->get_name() + "' initialization";
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

#include <OE/Carbon/exceptions_csl.h>
#include <OE/Events/event.h>
#include <OE/error_oe.h>
#include <OE/task_manager.h>
#include <OE/types/libs_oe.h>

using namespace std;

// This is where events' error handling is happening
oe::task_action oe::event_handler_t::call_event(size_t event_id) {

    /// generic event management
    auto            event  = events_list_[event_id];
    oe::task_action output = oe::task_action::discard;
    if (event.is_valid()) {

        event->lockMutex();
        try {
            output = event->call();
        } catch (oe::api_error& e) {
            std::string error_str = "[OE Error] '" + e.name_ + "' thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.get_counter()) + "\n";
            error_str += "\t" + e.what() + "\n";
            cout << error_str;
            OE_WriteToLog(error_str);
        } catch (csl::parser_error_t& e) {
            std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.get_counter()) + "\n";
            error_str += "\t" + e.what() + "\n";
            cout << error_str;
            OE_WriteToLog(error_str);
        } catch (csl::interpreter_error_t& e) {
            std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.get_counter()) + "\n";
            error_str += "\t" + e.what() + "\n";
            cout << error_str;
            OE_WriteToLog(error_str);
        } catch (std::exception& e) {
            std::string error_str = "[OE Error] std::exception variant thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.get_counter()) + "\n";
            error_str += "\t" + string(e.what()) + "\n";
            cout << error_str;
            OE_WriteToLog(error_str);
        } catch (...) {
            std::string error_str = "[OE Error] Exception thrown in event: '" + this->get_event_name(event->id) +
                                    "', event invocation counter: " + std::to_string(event->task_.get_counter());
            cout << error_str << endl;
            OE_WriteToLog(error_str + "\n");
        }
        event->unlockMutex();
    }
    else {
        // TODO: Warning
    }

    return output;
}

// error handling functions
int oe::task_manager_t::try_run_unsync_thread(oe::unsync_thread_data* actual_data) {

    oe::task_info_t unsync_task = oe::task_info_t(0, 0, actual_data->taskMgr->getTicks());
    unsync_task.set_type_task(oe::task_type::UNSYNC);
    try {
        actual_data->func(unsync_task);
        return 0;
    } catch (oe::api_error& e) {
        std::string error_str = "[OE Error] '" + e.name_ + "' thrown in unsync thread: '" + actual_data->name + "'" + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (csl::parser_error_t& e) {
        std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in unsync thread: '" + actual_data->name + "'" + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (csl::interpreter_error_t& e) {
        std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in unsync thread: '" + actual_data->name + "'" + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (oe::networking_error& e) {
        std::string error_str = "[SLC Error] '" + e.name_ + "' thrown in unsync thread: '" + actual_data->name + "'" + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (std::exception& e) {
        std::string error_str = "[OE Error] std::exception variant thrown in unsync thread: '" + actual_data->name + "'" + "\n";
        error_str += "\t" + string(e.what()) + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (...) {
        std::string error_str = "[OE Error] Exception thrown in unsync thread: '" + actual_data->name + "'";
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return 1;
}

oe::task_action oe::task_manager_t::try_run_task(std::size_t thread_id, std::shared_ptr<oe::task_t> task) {

    try {
        return task->execute(thread_id);
    } catch (oe::api_error& e) {
        std::string error_str = "[OE Error] '" + e.name_ + "' thrown in task: '" +
                                this->threads_[thread_id]->tasks_[task->id].get_name() + "', thread: '" +
                                this->threads_.get_name(thread_id);
        error_str += "', invocation: " + std::to_string(task->get_counter()) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (csl::parser_error_t& e) {
        std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in task: '" +
                                this->threads_[thread_id]->tasks_[task->id].get_name() + "', thread: '" +
                                this->threads_.get_name(thread_id);
        error_str += "', invocation: " + std::to_string(task->get_counter()) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (csl::interpreter_error_t& e) {
        std::string error_str = "[CSL Error] '" + e.name_ + "' thrown in task: '" +
                                this->threads_[thread_id]->tasks_[task->id].get_name() + "', thread: '" +
                                this->threads_.get_name(thread_id);
        error_str += "', invocation: " + std::to_string(task->get_counter()) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (std::exception& e) {
        std::string error_str = "[OE Error] '" + string(typeid(e).name()) + "' thrown in task: '" +
                                this->threads_[thread_id]->tasks_[task->id].get_name() + "', thread: '" +
                                this->threads_.get_name(thread_id);
        error_str += "', invocation: " + std::to_string(task->get_counter()) + "\n";
        error_str += "\t" + string(e.what()) + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (...) {
        /// universal error handling. will catch any exception
        /// feel free to add specific handling for specific errors
        string outputa =
            string("[OE Error] Exception thrown in task: '" + this->threads_[thread_id]->tasks_[task->id].get_name() +
                   "', thread: '" + this->threads_.get_name(thread_id));
        outputa += "', invocation: " + std::to_string(task->get_counter());
        cout << outputa << endl;
        OE_WriteToLog(outputa + "\n");
    }
    return oe::task_action::discard;
}

bool oe::task_manager_t::try_run_physics_update_multi_thread(std::size_t thread_id, const int& comp_threads_copy) {


    try {
        this->physics_->update_multi_thread(this->threads_[thread_id]->physics_task, comp_threads_copy);
        return false;
    } catch (oe::api_error& e) {
        std::string error_str = "[HPE Error] '" + e.name_ + "' thrown in update_multi_thread of '" + this->physics_->get_name() +
                                "' in  thread: '" + this->threads_.get_name(thread_id) +
                                "', thread_num: " + std::to_string(comp_threads_copy);
        error_str += ", invocation: " + std::to_string(this->threads_[thread_id]->physics_task.get_counter()) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (oe::physics_error& e) {
        std::string error_str = "[HPE Error] '" + e.name_ + "' thrown in update_multi_thread of '" + this->physics_->get_name() +
                                "' in  thread: '" + this->threads_.get_name(thread_id) +
                                "', thread_num: " + std::to_string(comp_threads_copy);
        error_str += ", invocation: " + std::to_string(this->threads_[thread_id]->physics_task.get_counter()) + "\n";
        error_str += "\t" + e.what() + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (std::exception& e) {
        std::string error_str = "[HPE Error] '" + string(typeid(e).name()) + "' thrown in update_multi_thread of '" +
                                this->physics_->get_name() + "' in  thread: '" + this->threads_.get_name(thread_id) +
                                "', thread_num: " + std::to_string(comp_threads_copy);
        error_str += ", invocation: " + std::to_string(this->threads_[thread_id]->physics_task.get_counter()) + "\n";
        error_str += "\t" + string(e.what()) + "\n";
        cout << error_str;
        OE_WriteToLog(error_str);
    } catch (...) {
        /// universal error handling. will catch any exception
        /// feel free to add specific handling for specific errors
        auto&  task    = this->threads_[thread_id]->physics_task;
        string outputa = string("[HPE Error] Physics exception thrown in update_multi_thread of '" + this->physics_->get_name() +
                                "' in  thread: '" + this->threads_.get_name(thread_id) +
                                "', thread_num: " + std::to_string(comp_threads_copy));
        outputa += ", invocation: " + std::to_string(task.get_counter());
        cout << outputa << endl;
        OE_WriteToLog(outputa + "\n");
    }
    return true;
}

bool oe::task_manager_t::try_run_renderer_update_single_thread() {

    this->renderer_mutex_.lockMutex();
    auto renderer_info_copy = this->renderer_info_;
    this->renderer_mutex_.unlockMutex();

    try {
        this->renderer_->update_single_thread(renderer_info_copy, this->window_output_);
        return false;
    } catch (oe::api_error& e) {
        std::string error_str = "[NRE Error] '" + e.name_ + "' thrown in update_single_thread of '" +
                                this->renderer_->get_name() + "', invocation: " + std::to_string(this->countar_);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (oe::renderer_error& e) {
        std::string error_str = "[NRE Error] '" + e.name_ + "' thrown in update_single_thread of '" +
                                this->renderer_->get_name() + "', invocation: " + std::to_string(this->countar_);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[NRE Error] '" + string(typeid(e).name()) + "' thrown in update_single_thread of '" +
                                this->renderer_->get_name() + "', invocation: " + std::to_string(this->countar_);
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[NRE Error] Renderer exception thrown in update_single_thread of '" +
                                this->renderer_->get_name() + "', invocation: " + std::to_string(this->countar_);
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

bool oe::task_manager_t::try_run_renderer_update_data() {

    this->renderer_mutex_.lockMutex();
    auto renderer_info_copy = this->renderer_info_;
    this->renderer_mutex_.unlockMutex();

    try {
        this->renderer_->update_data(renderer_info_copy, this->window_output_, this->restart_renderer_);
        return false;
    } catch (oe::api_error& e) {
        std::string error_str = "[NRE Error] '" + e.name_ + "' thrown in update_data of '" + this->renderer_->get_name() +
                                "', invocation: " + std::to_string(this->countar_);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (oe::renderer_error& e) {
        std::string error_str = "[NRE Error] '" + e.name_ + "' thrown in update_data of '" + this->renderer_->get_name() +
                                "', invocation: " + std::to_string(this->countar_);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[NRE Error] '" + string(typeid(e).name()) + "' thrown in update_data of '" +
                                this->renderer_->get_name() + "', invocation: " + std::to_string(this->countar_);
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[NRE Error] Renderer exception thrown in update_data of '" + this->renderer_->get_name() +
                                "', invocation: " + std::to_string(this->countar_);
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

bool oe::task_manager_t::try_run_winsys_update() {

    this->window_mutex_.lockMutex();
    auto winsys_info = this->window_info_;
    this->window_mutex_.unlockMutex();

    try {
        this->window_output_ = this->window_->update(winsys_info);
        return this->window_output_.done;
    } catch (oe::winsys_error& e) {
        std::string error_str =
            "[OE WINSYS Error] '" + e.name_ + "' thrown in update, invocation: " + std::to_string(this->countar_);
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[OE WINSYS Error] '" + string(typeid(e).name()) +
                                "' thrown in update, invocation: " + std::to_string(this->countar_);
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[OE WINSYS Error] Exception thrown in update, invocation: " + std::to_string(this->countar_);
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }

    return true;
}

bool oe::task_manager_t::try_run_winsys_init(int x, int y, std::string titlea, bool fullscreen, oe::winsys_init_info params) {

    this->window_init_info_           = params;
    this->window_info_.res_x          = x;
    this->window_info_.res_y          = y;
    this->window_info_.use_fullscreen = fullscreen;
    this->window_info_.title          = titlea;

    try {
        this->window_output_ = this->window_->init(this->window_init_info_, this->window_info_);
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

bool oe::task_manager_t::try_run_physics_init(oe::physics_init_info params) {

    try {
        this->physics_init_info_ = params;
        this->physics_->init(params);
        return false;
    } catch (oe::physics_error& e) {
        std::string error_str =
            "[HPE Error] '" + e.name_ + "' thrown in physics engine '" + this->physics_->get_name() + "' initialization";
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[HPE Error] '" + string(typeid(e).name()) + "' thrown in physics engine '" +
                                this->physics_->get_name() + "' initialization";
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[HPE Error] Could not initialize physics engine '" + this->physics_->get_name() +
                                "' due to thrown exception in physics->init()";
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

bool oe::task_manager_t::try_run_renderer_init(oe::renderer_init_info params) {

    try {
        this->renderer_init_info_ = params;
        this->renderer_->init(this->renderer_init_info_, this->renderer_info_, this->window_output_);
        return false;
    } catch (oe::renderer_error& e) {
        std::string error_str =
            "[NRE Error] '" + e.name_ + "' thrown in renderer '" + this->renderer_->get_name() + "' initialization";
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[NRE Error] '" + string(typeid(e).name()) + "' thrown in renderer '" +
                                this->renderer_->get_name() + "' initialization";
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[NRE Error] Could not initialize renderer due to thrown exception in renderer '" +
                                this->renderer_->get_name() + "' initialization";
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

bool oe::task_manager_t::try_run_network_init(oe::networking_init_info params) {
    try {
        this->network_init_info_ = params;
        this->network_->init(params);
        return false;
    } catch (oe::networking_error& e) {
        std::string error_str =
            "[SLC Error] '" + e.name_ + "' thrown in networking '" + this->network_->get_name() + "' initialization";
        error_str += "\n\t" + e.what();
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (std::exception& e) {
        std::string error_str = "[SLC Error] '" + string(typeid(e).name()) + "' thrown in networking '" +
                                this->network_->get_name() + "' initialization";
        error_str += "\n\t" + string(e.what());
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    } catch (...) {
        std::string error_str = "[SLC Error] Could not initialize networking due to thrown exception in networking '" +
                                this->network_->get_name() + "' initialization";
        cout << error_str << endl;
        OE_WriteToLog(error_str + "\n");
    }
    return true;
}

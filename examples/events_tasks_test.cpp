#include <OE/api_oe.h>

int move_camera(oe::task task, std::string obj_name){

    if (oe::is_mouse_moved() && oe::is_mouse_locked()){
        float x = (float)oe::get_delta_mouse_x()/160.0f;
        float y = (float)oe::get_delta_mouse_y()/160.0f;

        oe::change_object_global_rot(obj_name, oe::vec4(-x, 0.0f, 0.0f, 1.0f));
        oe::change_object_rot(obj_name, oe::vec4(-y, 1.0f, 0.0f, 0.0f));
    }

    return task.CONTINUE();
}

std::unordered_map<std::string, int> dummy_list;
std::unordered_set<std::size_t> event_ids;

int broadcast_all_events_test(oe::task task){

    for (std::size_t event_id : event_ids){
        oe::broadcast_event(event_id);
    }

    return task.CONTINUE();
}

int test_event_func(oe::task, std::string event_name){

    dummy_list[event_name]++;
    return 0;
}

int OnloadVerySimple(oe::task load_event_task, std::string event_name){

    oe::add_task_func("move_camera", &move_camera, "Camera");

    for (int i=0; i<4000; i++){
        std::size_t event_id = oe::create_event("test_event" + std::to_string(i));
        oe::set_event_func("test_event" + std::to_string(i), &test_event_func);

        dummy_list["test_event" + std::to_string(i)] = 0;
        event_ids.insert(event_id);
    }

    oe::add_task_func("broadcast_events_task", &broadcast_all_events_test);

    oe::mouse_lock();
    return 0;
}

int main(){

    oe::preinit::request_gles2();
    oe::init("Events and Tasks test", 852, 480, false);
    oe::load_world_func("assets/OE_VerySimple.csl", &OnloadVerySimple);

    oe::start();
    return 0;
};

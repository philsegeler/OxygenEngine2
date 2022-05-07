#include <OE/api_oe.h>

int move_camera(oe::task_t task, std::string obj_name){

    size_t obj_id = oe::get_object_id(obj_name);
    if (oe::is_mouse_moved() && oe::is_mouse_locked()){
        float x = (float)oe::get_delta_mouse_x()/160.0f;
        float y = (float)oe::get_delta_mouse_y()/160.0f;

        oe::change_object_global_rot(obj_id, oe::vec4(-x, 0.0f, 0.0f, 1.0f));
        oe::change_object_rot(obj_id, oe::vec4(-y, 1.0f, 0.0f, 0.0f));
    }

    return 0;
}

int toggle_mouse_locked_state( oe::task_t, std::size_t event_id){

    if (oe::is_mouse_locked()){
        oe::mouse_unlock();
    } else {
        oe::mouse_lock();
    }
    return 0;
}

std::unordered_map<std::string, int> dummy_list;
std::unordered_set<std::size_t> event_ids;

int broadcast_all_events_test(oe::task_t task){

    for (std::size_t event_id : event_ids){
        oe::broadcast_event(event_id);
    }

    return 0;
}

int test_event_func(oe::task_t, std::size_t event_id){

    dummy_list[oe::get_event_name(event_id)]++;
    return 0;
}

int OnloadVerySimple(oe::task_t load_event_task, std::size_t event_id){


    // Movement tasks/events
    oe::add_task_func("move_camera", &move_camera, "Camera");
    oe::set_event_func("keyboard-space+", &toggle_mouse_locked_state);

    // Actual event tests
    // Simulate 42000 Events produced from any source for each frame
    // Only 10000 of them are activated each frame
    // I choose 42000 because 200 objects each collides with each and 2000 for the user
    for (int i=0; i<42000; i++){
        std::size_t event_id = oe::create_event("test_event" + std::to_string(i));

        // The user has a callback for only the first 10000 each frame. I believe this is a realistic
        // assumption, considering I have no idea how many more events we will generate internally
        // and I definitely do not believe the user will handle ALL of them.
        // If the user does not handle them, then they are still registered,
        // but their std::function is not called
        if (i < 1000){
            oe::set_event_func("test_event" + std::to_string(i), &test_event_func);
            dummy_list["test_event" + std::to_string(i)] = 0;
        }
        if (i < 10000)
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

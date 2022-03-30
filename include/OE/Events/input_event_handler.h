#ifndef OE_INPUTEVENTHANDLER_H
#define OE_INPUTEVENTHANDLER_H

#include <OE/Events/event.h>


/** This class is intended to store any user inputs
 *
 */
namespace oe{
    class input_event_handler_t {
    public:
        friend class OE_EventHandler;
        friend class OE_TaskManager;

        input_event_handler_t();
        ~input_event_handler_t();

        void createEvents(std::map<std::string, std::shared_ptr<oe::event_t>>*);

        // protected:
        std::vector<std::string>                 active_events;
        std::unordered_map<uint8_t, std::string> mouseList;
        ;

        std::unordered_map<int, std::string> keyList;
    };
};
#endif // OE_INPUTEVENTHANDLER_H

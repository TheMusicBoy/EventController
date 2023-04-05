#ifndef _ECL_CONTROLLER_HPP_
#define _ECL_CONTROLLER_HPP_

#include <ECL/event_queue.hpp>
#include <ECL/handler.hpp>
#include <ECL/utils.hpp>
#include <list>
#include <mutex>

namespace ec {

////////////////////////////////////////////////////////////
/// \brief Base class of object that used to control event
/// loop.
////////////////////////////////////////////////////////////
template <typename Data>
class ControllerBase {
 protected:
    using Container = HandlerList<Data>;
    using EQueue    = EventQueue<Data>;

    std::mutex lock_;
    Container handler_list_;
    EQueue event_queue_;

 public:
    ControllerBase() = default;
    ~ControllerBase() {}

    void loadEvents(EQueue* event_queue) {
        event_queue_.splice(*event_queue);
    }

    void call() {
        Data data;

        while(event_queue_.pollEvent(&data))
            handler_list_.call(data);
    }

    virtual void process() = 0;
};

}  // namespace ec

#endif
#ifndef _ECL_EVENT_QUEUE_HPP_
#define _ECL_EVENT_QUEUE_HPP_

#include <ECL/impl/ts_list.hpp>
#include <list>
#include <mutex>

namespace ec {

////////////////////////////////////////////////////////////
/// \brief Class that contains events and helps to operate
/// with it.
////////////////////////////////////////////////////////////
template <typename Data>
class EventQueue : protected TsList<Data> {
 protected:
    using Self = EventQueue<Data>;
    using Base = TsList<Data>;

    using Position = typename Base::Position;

    using Base::resource_;
    using Base::lock_;

 public:
    EventQueue()             = default;
    EventQueue(const Self&)  = delete;
    EventQueue(Self&& other) = default;
    ~EventQueue() = default;

    Self& operator=(Self&& other) {
        return this->Base::operator=(std::move(other));
    }

    bool pollEvent(Data* data) {
        std::lock_guard lock(lock_);
        if (resource_.empty()) return false;

        *data = resource_.front();
        resource_.pop_front();
        return true;
    }

    void splice(Self& other) {
        this->Base::splice(resource_.end(), other);
    }

    void push(const Data& data) {
        resource_.emplace_back(data);
    }

    void clear() {
        resource_.clear();
    }
};

}  // namespace ec

#endif
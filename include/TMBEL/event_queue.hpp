#ifndef _TMBEL_EVENT_QUEUE_HPP_
#define _TMBEL_EVENT_QUEUE_HPP_

#include <list>
#include <mutex>

namespace ec {

////////////////////////////////////////////////////////////
/// \brief Class that contains events and helps to operate
/// with it.
////////////////////////////////////////////////////////////
template <typename Data>
class EventQueue {
 protected:
    using Self      = EventQueue<Data>;
    using Container = std::list<Data>;
    using Position  = typename Container::iterator;

    std::mutex lock_;
    Container resource_;

 public:
    EventQueue()                  = default;
    EventQueue(const Self&) = delete;
    EventQueue(Self&& other) { *this = std::move(other); }
    ~EventQueue() { std::lock_guard lock(lock_); }

    Self& operator=(Self&& other) {
        other.lock_.lock();
        this->lock_.lock();

        resource_ = std::move(other.resource_);

        this->lock_.unlock();
        other.lock_.unlock();
    }

    bool pollEvent(Data* data) {
        std::lock_guard lock(lock_);
        if (resource_.empty()) return false;

        *data = resource_.front();
        resource_.pop_front();
        return true;
    }

    void splice(Self& other) {
        other.lock_.lock();
        this->lock_.lock();

        resource_.splice(resource_.end(), other.resource_);

        this->lock_.unlock();
        other.lock_.unlock();
    }

    void push(const Data& data) {
        std::lock_guard lock(lock_);
        resource_.emplace_back(data);
    }

    void clear() {
        std::lock_guard lock(lock_);
        resource_.clear();
    }
};

}  // namespace ec

#endif
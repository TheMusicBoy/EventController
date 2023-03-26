#ifndef _TMBEL_GLOBAL_CONTAINER_HPP_
#define _TMBEL_GLOBAL_CONTAINER_HPP_

#include <TMBEL/handler.hpp>
#include <list>
#include <map>
#include <mutex>
#include <vector>

namespace ec {

////////////////////////////////////////////////////////////
/// \brief Base class of objects that contains handlers with
/// global access.
////////////////////////////////////////////////////////////

template <typename Index>
class GlobalMapBase {
 protected:
    using Container = std::map<Index, HandlerListBase*>;
    using Position = Index;
    using HandlerPos = std::list<HandlerListBase*>::iterator;

    Container resource_;
    std::recursive_mutex lock_;

    GlobalMapBase() {}

 public:
    HandlerListBase* get(Position index) {
        std::lock_guard lock(lock_);
        return resource_.at(index);
    }

    Position push(Position index, HandlerListBase* handler) {
        std::lock_guard lock(lock_);
        return resource_.insert(std::make_pair(index, handler));
    }

    HandlerListBase* pop(Position index) {
        std::lock_guard lock(lock_);
        HandlerListBase* handler = resource_.at(index);
        resource_.erase(index);
        return handler;
    }

};

class GlobalMasBase {
protected:
    using Container = std::vector<HandlerListBase*>;
    using Position = size_t;

    Container resource_;
    std::recursive_mutex lock_;

    GlobalMasBase() : resource_(0, nullptr) {}

 public:
    void setCount(Position new_count) {
        resource_.resize(new_count, nullptr);
    }

    HandlerListBase* get(Position index) {
        std::lock_guard lock(lock_);
        return resource_.at(index);
    }

    Position push(HandlerListBase* handler) {
        std::lock_guard lock(lock_);

        resource_.push_back(handler);
        return resource_.size() - 1;
    }

    Position push(Position index, HandlerListBase* handler) {
        std::lock_guard lock(lock_);
        if (resource_.at(index) != nullptr)
            throw;
        
        resource_.at(index) = handler;
        return index;
    }

    HandlerListBase* pop(Position index) {
        std::lock_guard lock(lock_);
        if (resource_.at(index) == nullptr)
            throw;

        HandlerListBase* handler = resource_.at(index);
        resource_.at(index) = nullptr;
        return handler;
    }
};

class GlobalListBase {
 protected:
    using Container = std::list<HandlerListBase*>;
    using Position = typename Container::iterator;

    Container resource_;
    std::recursive_mutex lock_;

    GlobalListBase() {}

 public:
    HandlerListBase* get(Position index) {
        std::lock_guard lock(lock_);
        return *index;
    }

    Position push(Position index, HandlerListBase* handler) {
        std::lock_guard lock(lock_);
        resource_.push_back(handler);
        return std::prev(resource_.end());
    }

    HandlerListBase* pop(Position index) {
        std::lock_guard lock(lock_);
        HandlerListBase* handler = *index;
        resource_.erase(index);
        return handler;
    }
};

}  // namespace ec

#endif
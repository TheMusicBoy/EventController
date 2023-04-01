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

template <typename Index, typename Data>
class GlobalMapBase {
 protected:
    using Object       = Handler<Data>;
    using ObjContainer = HandlerList<Data>;
    using Container    = std::map<Index, ObjContainer*>;
    using Position     = Index;

    using HandlerPos = typename ObjContainer::Position;

    Container resource_;
    std::recursive_mutex lock_;


 public:
    GlobalMapBase() = default;

    ObjContainer* get(Position index) {
        std::lock_guard lock(lock_);
        return resource_.at(index);
    }

    Position push(Position index, ObjContainer* handler) {
        std::lock_guard lock(lock_);
        return resource_.insert(std::make_pair(index, handler));
    }

    ObjContainer* pop(Position index) {
        std::lock_guard lock(lock_);
        ObjContainer* handler = resource_.at(index);
        resource_.erase(index);
        return handler;
    }
};

template <typename Data>
class GlobalMasBase {
 protected:

    using Object       = Handler<Data>;
    using ObjContainer = HandlerList<Data>;
    using Container    = std::vector<ObjContainer*>;
    using Position     = uint32_t;

    using HandlerPos = typename ObjContainer::Position;

    Container resource_;
    std::recursive_mutex lock_;


 public:
    GlobalMasBase() : resource_(0, nullptr) {}

    void setCount(Position new_count) { resource_.resize(new_count, nullptr); }

    ObjContainer* get(Position index) {
        std::lock_guard lock(lock_);
        return resource_.at(index);
    }

    Position push(ObjContainer* handler) {
        std::lock_guard lock(lock_);

        resource_.push_back(handler);
        return resource_.size() - 1;
    }

    Position push(Position index, ObjContainer* handler) {
        std::lock_guard lock(lock_);
        if (resource_.at(index) != nullptr) throw;

        resource_.at(index) = handler;
        return index;
    }

    ObjContainer* pop(Position index) {
        std::lock_guard lock(lock_);
        if (resource_.at(index) == nullptr) throw;

        ObjContainer* handler = resource_.at(index);
        resource_.at(index)   = nullptr;
        return handler;
    }
};

template <typename Data>
class GlobalListBase {
 protected:
    using Object       = Handler<Data>;
    using ObjContainer = HandlerList<Data>;
    using Container    = std::list<ObjContainer*>;
    using Position     = typename Container::iterator;

    using HandlerPos = typename ObjContainer::Position;

    Container resource_;
    std::recursive_mutex lock_;


 public:
    GlobalListBase() = default;

    ObjContainer* get(Position index) {
        std::lock_guard lock(lock_);
        return *index;
    }

    Position push(Position index, ObjContainer* handler) {
        std::lock_guard lock(lock_);
        resource_.push_back(handler);
        return std::prev(resource_.end());
    }

    ObjContainer* pop(Position index) {
        std::lock_guard lock(lock_);
        ObjContainer* handler = *index;
        resource_.erase(index);
        return handler;
    }
};

}  // namespace ec

#endif
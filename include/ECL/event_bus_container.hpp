#ifndef _ECL_EVENT_BUS_CONTAINER_HPP_
#define _ECL_EVENT_BUS_CONTAINER_HPP_

#include <ECL/handler.hpp>
#include <list>
#include <map>
#include <shared_mutex>
#include <vector>

namespace ec {

////////////////////////////////////////////////////////////
/// \brief Base class of Event Bus objects.
////////////////////////////////////////////////////////////
template <typename Index, typename Data>
class EventBusContainer {
 protected:
    using Object       = Handler<Data>;
    using ObjContainer = HandlerList<Data>;
    using ObjPosition  = typename ObjContainer::Position;
    using Position     = Index;

 public:
    virtual ObjContainer* get(Position index)                   = 0;
    virtual Position push(Position index, ObjContainer* object) = 0;
    virtual ObjContainer* pop(Position index)                   = 0;

    ObjPosition attach(Position index, Object* object) {
        return object->attachTo(this->get(index));
    }
};

////////////////////////////////////////////////////////////
/// \brief Base class of objects that contains handlers with
/// fast access.
////////////////////////////////////////////////////////////

template <typename Index, typename Data>
class EventBusMap : public EventBusContainer<Index, Data> {
 protected:
    using Self = EventBusMap<Index, Data>;
    using Base = EventBusContainer<Index, Data>;

    using Object       = typename Base::Object;
    using ObjContainer = typename Base::ObjContainer;
    using ObjPosition  = typename Base::ObjPosition;
    using Position     = typename Base::Position;

    using Container = std::map<Index, ObjContainer*>;

    std::shared_mutex lock_;
    Container resource_;

 public:
    EventBusMap() = default;

    ObjContainer* get(Position index) override {
        std::shared_lock lock(lock_);
        return resource_.at(index);
    }

    Position push(Position index, ObjContainer* handler) override {
        std::unique_lock lock(lock_);
        return resource_.insert(std::make_pair(index, handler));
    }

    ObjContainer* pop(Position index) override {
        std::unique_lock lock(lock_);
        ObjContainer* handler = resource_.at(index);
        resource_.erase(index);
        return handler;
    }
};

template <typename Data>
class EventBusMas : public EventBusContainer<size_t, Data> {
 protected:
    using Self = EventBusMas<Data>;
    using Base = EventBusContainer<size_t, Data>;

    using Object       = typename Base::Object;
    using ObjContainer = typename Base::ObjContainer;
    using ObjPosition  = typename Base::ObjPosition;
    using Position     = typename Base::Position;

    using Container = std::vector<ObjContainer*>;

    Container resource_;
    std::shared_mutex lock_;

 public:
    EventBusMas() : resource_(0, nullptr) {}

    void setCount(Position new_count) { resource_.resize(new_count, nullptr); }

    ObjContainer* get(Position index) override {
        std::shared_lock lock(lock_);
        return resource_.at(index);
    }

    Position push(ObjContainer* handler) {
        std::unique_lock lock(lock_);

        resource_.push_back(handler);
        return resource_.size() - 1;
    }

    Position push(Position index, ObjContainer* handler) override {
        std::unique_lock lock(lock_);
        if (resource_.at(index) != nullptr) throw;

        resource_.at(index) = handler;
        return index;
    }

    ObjContainer* pop(Position index) override {
        std::unique_lock lock(lock_);
        if (resource_.at(index) == nullptr) throw;

        ObjContainer* handler = resource_.at(index);
        resource_.at(index)   = nullptr;
        return handler;
    }
};

}  // namespace ec

#endif
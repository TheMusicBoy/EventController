#ifndef _TMBEL_UTILS_HPP_
#define _TMBEL_UTILS_HPP_

#include <TMBEL/global_container.hpp>
#include <TMBEL/handler.hpp>
#include <list>
#include <mutex>

namespace ec {



////////////////////////////////////////////////////////////
/// \brief Container that can contain handler and deletes it
/// when call destructor.
////////////////////////////////////////////////////////////
class UniqueContainer {
 protected:
    using Container = std::list<HandlerBase*>;
    using Position  = typename Container::iterator;

    std::recursive_mutex lock_;
    Container resource_;

 public:
    UniqueContainer()                       = default;
    UniqueContainer(const UniqueContainer&) = delete;
    UniqueContainer(UniqueContainer&&);
    ~UniqueContainer();

    Position push(HandlerBase* handler);

    HandlerBase* pop(Position position);
    void del(Position position);
};

////////////////////////////////////////////////////////////
// Containers
////////////////////////////////////////////////////////////

template <typename Data>
std::list<HandlerBase*>::iterator asyncHandler(
    ObsObjectBase<Handler<Data>>* container, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return handler->attachTo(container);
}

template <typename Data>
std::list<HandlerBase*>::iterator asyncHandler(
    ObsObjectBase<Handler<Data>>* container, std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return handler->attachTo(position, container);
}

template <typename Data>
std::list<HandlerBase*>::iterator syncHandler(
    ObsObjectBase<Handler<Data>>* container, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return handler->attachTo(container);
}

template <typename Data>
std::list<HandlerBase*>::iterator syncHandler(
    ObsObjectBase<Handler<Data>>* container, std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return handler->attachTo(position, container);
}

}  // namespace ec

#endif
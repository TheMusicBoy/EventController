#ifndef _ECL_UTILS_HPP_
#define _ECL_UTILS_HPP_

#include <ECL/ts_list.hpp>
#include <ECL/handler.hpp>
#include <list>
#include <mutex>

namespace ec {



////////////////////////////////////////////////////////////
/// \brief Container that can contain handler and deletes it
/// when call destructor.
////////////////////////////////////////////////////////////
class UniqueContainer : protected TsList<HandlerBase*> {
 protected:
    using Self = UniqueContainer;
    using Base = TsList<HandlerBase*>;

    using Position  = typename Base::Position;

 public:
    UniqueContainer();
    UniqueContainer(const Self&) = delete;
    UniqueContainer(Self&&);
    ~UniqueContainer();

    Position push(HandlerBase* handler);

    HandlerBase* pop(Position position);
    void del(Position position);
};

////////////////////////////////////////////////////////////
// Containers
////////////////////////////////////////////////////////////

template <typename Data>
typename HandlerList<Data>::Position asyncHandler(
    HandlerList<Data>* container, std::function<void(const Data&)>&& function) {
    Handler<Data>* handler = new AsyncFuncHandler<Data>(std::move(function));
    return container->attach(handler);
}

template <typename Data>
typename HandlerList<Data>::Position asyncHandler(
    HandlerList<Data>* container, std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)>&& function) {
    Handler<Data>* handler = new AsyncFuncHandler<Data>(std::move(function));
    return container->attach(handler);
}

template <typename Data>
typename HandlerList<Data>::Position syncHandler(
    HandlerList<Data>* container, std::function<void(const Data&)>&& function) {
    Handler<Data>* handler = new SyncFuncHandler<Data>(std::move(function));
    return container->attach(handler);
}

template <typename Data>
typename HandlerList<Data>::Position syncHandler(
    HandlerList<Data>* container, std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)>&& function) {
    Handler<Data>* handler = new SyncFuncHandler<Data>(std::move(function));
    return container->attach(handler);
}

}  // namespace ec

#endif
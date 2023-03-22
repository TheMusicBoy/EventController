#ifndef _TMBEL_UTILS_HPP_
#define _TMBEL_UTILS_HPP_

#include <TMBEL/handler.hpp>
#include <mutex>
#include <list>

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
    UniqueContainer() = default;
    UniqueContainer(const UniqueContainer&) = delete;
    UniqueContainer(UniqueContainer&&);
    ~UniqueContainer();

    Position push(HandlerBase* handler);

    HandlerBase* pop(Position position);
    void del(Position position);
};

template<typename Data>
std::list<HandlerBase*>::iterator asyncHandler(HandlerListBase* container, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return handler->attach(static_cast<HandlerList<Data>*>(container));
}

template<typename Data>
std::list<HandlerBase*>::iterator asyncHandler(HandlerListBase* container, std::list<HandlerBase*>::iterator position, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return handler->attach(position, static_cast<HandlerList<Data>*>(container));
}

template<typename Data>
std::list<HandlerBase*>::iterator syncHandler(HandlerListBase* container, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return handler->attach(static_cast<HandlerList<Data>*>(container));
}

template<typename Data>
std::list<HandlerBase*>::iterator syncHandler(HandlerListBase* container, std::list<HandlerBase*>::iterator position, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return handler->attach(position, static_cast<HandlerList<Data>*>(container));
}

}  // namespace ec

#endif
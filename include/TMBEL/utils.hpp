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
    HandlerListBase* container, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return handler->attach(container);
}

template <typename Data>
std::list<HandlerBase*>::iterator asyncHandler(
    HandlerListBase* container, std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return handler->attach(position, container);
}

template <typename Data>
std::list<HandlerBase*>::iterator syncHandler(
    HandlerListBase* container, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return handler->attach(container);
}

template <typename Data>
std::list<HandlerBase*>::iterator syncHandler(
    HandlerListBase* container, std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return handler->attach(position, container);
}

////////////////////////////////////////////////////////////
// Parsers
////////////////////////////////////////////////////////////

template <typename Data>
std::list<HandlerBase*>::iterator asyncHandler(
    HandlerBase* container, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return static_cast<ParserBase<Data>*>(container)->push(0, handler);
}

template <typename Data>
std::list<HandlerBase*>::iterator asyncHandler(
    HandlerBase* container, std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return static_cast<ParserBase<Data>*>(container)->push(0, position,
                                                           handler);
}

template <typename Data>
std::list<HandlerBase*>::iterator syncHandler(
    HandlerBase* container, std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return static_cast<ParserBase<Data>*>(container)->push(0, handler);
}

template <typename Data>
std::list<HandlerBase*>::iterator syncHandler(
    HandlerBase* container, std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return static_cast<ParserBase<Data>*>(container)->push(0, position,
                                                           handler);
}

template <typename Data>
std::list<HandlerBase*>::iterator asyncHandler(
    HandlerBase* container, size_t group,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return static_cast<ParserBase<Data>*>(container)->push(group, handler);
}

template <typename Data>
std::list<HandlerBase*>::iterator asyncHandler(
    HandlerBase* container, size_t group,
    std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new AsyncFuncHandler(function);
    return static_cast<ParserBase<Data>*>(container)->push(group, position,
                                                           handler);
}

template <typename Data>
std::list<HandlerBase*>::iterator syncHandler(
    HandlerBase* container, size_t group,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return static_cast<ParserBase<Data>*>(container)->push(group, handler);
}

template <typename Data>
std::list<HandlerBase*>::iterator syncHandler(
    HandlerBase* container, size_t group,
    std::list<HandlerBase*>::iterator position,
    std::function<void(const Data&)> function) {
    Handler<Data>* handler = new SyncFuncHandler(function);
    return static_cast<ParserBase<Data>*>(container)->push(group, position,
                                                           handler);
}

}  // namespace ec

#endif
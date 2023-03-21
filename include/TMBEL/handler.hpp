#ifndef _TMBEL_EVENT_HANDLER_HPP_
#define _TMBEL_EVENT_HANDLER_HPP_

#include <functional>
#include <list>
#include <mutex>
#include <vector>

namespace ev {

class HandlerListBase;

class HandlerBase {
 protected:
    using Container = HandlerListBase;
    using Position  = typename std::list<HandlerBase*>::iterator;

 private:
    Container* container_;
    Position position_;

 public:
    HandlerBase();
    HandlerBase(Container* container);
    HandlerBase(Container* container, Position position);
    ~HandlerBase();

    void attach(Container* container);
    void attach(Container* container, Position position);
    void detach();

    virtual void remove();
};

class HandlerListBase {
 protected:
    using HandlerList = std::list<HandlerBase*>;
    using HandlerPos  = typename HandlerList::iterator;

    std::recursive_mutex lock_;
    HandlerList resource_;

 public:
    HandlerListBase();
    HandlerListBase(const HandlerListBase&) = delete;
    HandlerListBase(HandlerListBase&&);
    ~HandlerListBase();

    HandlerPos push(HandlerBase* handler);
    HandlerPos insert(HandlerPos position, HandlerBase* handler);

    HandlerBase* pop(HandlerPos position);

    void clear();

    bool empty();
    size_t size();
};


}  // namespace ev

#endif
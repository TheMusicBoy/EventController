#ifndef _TMBEL_HANDLER_HPP_
#define _TMBEL_HANDLER_HPP_

#include <functional>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

namespace ec {

class HandlerListBase;

////////////////////////////////////////////////////////////
/// \brief Base class of all handlers.
////////////////////////////////////////////////////////////
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

    Position attach(Container* container);
    Position attach(Container* container, Position position);
    void detach();

    virtual void remove();
};

////////////////////////////////////////////////////////////
/// \brief Base class of handler container that safe for
/// multithread work.
////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////
/// \brief Base class of handler that receive Data object
////////////////////////////////////////////////////////////
template <typename Data>
class Handler : public HandlerBase {
 protected:
    using Self = Handler<Data>;
    using Base = HandlerBase;

 public:
    Handler() = default;
    Handler(Container* container) : Base(container) {}
    Handler(Container* container, Position position)
        : Base(container, position) {}

    virtual void call(const Data& data) = 0;
};

////////////////////////////////////////////////////////////
/// \brief Class that can contain handlers and call them
/// save for multithread work.
////////////////////////////////////////////////////////////
template <typename Data>
class HandlerList : public HandlerListBase {
 protected:
    using Self = HandlerList<Data>;
    using Base = HandlerListBase;
    using El   = Handler<Data>;

 public:
    HandlerList() = default;
    HandlerList(HandlerList&& other) : Base(other) {}

    void call(const Data& data) {
        std::lock_guard lock(lock_);
        for (auto& el : resource_) static_cast<El*>(el)->call(data);
    }
};

////////////////////////////////////////////////////////////
/// \brief Handler that process and transforms data to
/// another type by specified operation.
////////////////////////////////////////////////////////////
template <typename Data, typename Result>
class Processor : public Handler<Data> {
 protected:
    using Self = Processor<Data, Result>;
    using Base = Handler<Data>;

    using Container = typename Base::Container;
    using Position  = typename Base::Position;

    using HandlerObj = Handler<Result>;
    using List       = HandlerList<Data>;
    using HandlerPos = typename List::iterator;
    using Process    = std::function<Result(const Data&)>;

    Process process_;
    List resource_;

 public:
    Processor() = default;
    Processor(Container* container) : Base(container) {}
    Processor(Container* container, Position position)
        : Base(container, position) {}
    Processor(Process process) : process_(process) {}

    void setProcess(Process process) { process_ = process; }

    void call(const Data& data) override {
        Result result = process_(data);
        resource_.call(result);
    }

    HandlerPos push(HandlerObj* object) { return resource_.push(object); }

    HandlerPos insert(HandlerPos position, HandlerObj* object) {
        return resource_.insert(position, object);
    }
};

////////////////////////////////////////////////////////////
/// \brief Base class of all parsers used to split data for
/// handlers by some algorithm.
////////////////////////////////////////////////////////////
template <typename Data>
class ParserBase : virtual public Handler<Data> {
 protected:
    using Self = ParserBase<Data>;
    using Base = Handler<Data>;

    using Container = typename Base::Container;
    using Position  = typename Base::Position;

    using HandlerObj = Handler<Data>;
    using List       = HandlerList<Data>;
    using HandlerPos = typename List::iterator;

    std::vector<List> resource_;

 public:
    ParserBase() = default;
    ParserBase(size_t group_count) : resource_(group_count) {}

    void setGroupCount(size_t group) { resource_.resize(group); }

    HandlerPos push(size_t group, HandlerObj* handler) {
        return resource_[group].push(handler);
    }

    HandlerPos insert(size_t group, HandlerPos position, HandlerObj* handler) {
        return resource_[group].insert(position, handler);
    }
};

////////////////////////////////////////////////////////////
/// \brief Handler that can call function that will be set.
////////////////////////////////////////////////////////////
template <typename Data>
class SyncFuncHandler : public Handler<Data> {
 protected:
    using Self = SyncFuncHandler<Data>;
    using Base = Handler<Data>;

    using Container = typename Base::Container;
    using Position  = typename Base::Position;

    using Func = std::function<void(const Data&)>;

    std::recursive_mutex lock_;
    Func function_;

 public:
    SyncFuncHandler() = default;
    SyncFuncHandler(Func function) : Base() {
        function_ = function;
    }

    void setFunction(Func function) {
        std::lock_guard lock(lock_);
        function_ = function;
    }

    void call(const Data& data) override {
        std::lock_guard lock(lock_);
        if (function_) function_(data);
    }
};

////////////////////////////////////////////////////////////
/// \brief Handler that can asynchronously call function 
/// that will be set.
////////////////////////////////////////////////////////////
template <typename Data>
class AsyncFuncHandler : public Handler<Data> {
 protected:
    using Self = AsyncFuncHandler<Data>;
    using Base = Handler<Data>;

    using Container  = typename Base::Container;
    using Position   = typename Base::Position;
    using ThreadList = std::list<std::thread>;

    using Func = std::function<void(const Data&)>;

    std::recursive_mutex lock_;
    Func function_;
    ThreadList threads_;

 public:
    AsyncFuncHandler() = default;
    AsyncFuncHandler(Func function) : Base() {
        function_ = function;
    }
    ~AsyncFuncHandler() {
        std::lock_guard lock(lock_);
        for (auto& el : threads_) el.join();
    }

    void setFunction(Func function) {
        std::lock_guard lock(lock_);
        function_ = function;
    }

    void call(const Data& data) override {
        std::lock_guard lock(lock_);
        if (function_) threads_.emplace_back(function_, std::ref(data));
    }
};

}  // namespace ec

#endif
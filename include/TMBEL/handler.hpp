#ifndef _TMBEL_HANDLER_HPP_
#define _TMBEL_HANDLER_HPP_

#include <TMBEL/lock_handler.hpp>
#include <TMBEL/multithread_list.hpp>
#include <TMBEL/process_list.hpp>
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
class HandlerBase : public SubObjectBase<HandlerBase> {
 private:
    using Self = HandlerBase;
    using Base = SubObjectBase<Self>;

 public:
    HandlerBase();
    HandlerBase(Container* container);
    HandlerBase(Position position, Container* container);
    virtual ~HandlerBase() = default;

    virtual void onRemove();
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
    Handler(Position position, Container* container)
        : Base(position, container) {}
    virtual ~Handler() override = default;

    virtual void call(const Data& data) = 0;
};

////////////////////////////////////////////////////////////
/// \brief Handler that process and transforms data to
/// another type by specified operation.
////////////////////////////////////////////////////////////
template <typename Data, typename Result>
class Processor : public ObsObjectBase<Handler<Result>>,
                  virtual public Handler<Data> {
 protected:
    using Self    = Processor<Data, Result>;
    using SubBase = Handler<Data>;
    using ObsBase = ObsObjectBase<Handler<Result>>;

    using Sub     = Handler<Result>;
    using Process = std::function<Result(const Data&)>;

    Process process_;

private:
    using Container = typename SubBase::Container;
    using Position = typename SubBase::Position;
    
    using ObsBase::sub_list_;

 public:
    Processor() = default;
    Processor(Container* container) : SubBase(container) {}
    Processor(Position position, Container* container)
        : SubBase(position, container) {}
    Processor(Process process) : process_(process) {}
    virtual ~Processor() override = default;

    void setProcess(Process process) { process_ = process; }

    void call(const Data& data) override {
        Result result = process_(data);
        sub_list_.map([result](Sub* handler) { handler->call(result); });
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

    using Object = Handler<Data>;
    using List   = ObsObjectBase<Object>;

    std::vector<List> resource_;

 public:
    using Position = typename List::Position;

    ParserBase() = default;
    ParserBase(size_t group_count) : resource_(group_count) {}
    virtual ~ParserBase() override = default;

    void setGroupCount(size_t group) { resource_.resize(group); }

    Position attach(size_t group, Object* handler) {
        return resource_[group].attach(handler);
    }

    Position attach(size_t group, Position position, Object* handler) {
        return resource_[group].attach(position, handler);
    }
};

////////////////////////////////////////////////////////////
/// \brief Base of class that will execute some function,
////////////////////////////////////////////////////////////
template <typename Data>
class FuncHandlerBase : public Handler<Data> {
 private:
    using Self = FuncHandlerBase;
    using Base = Handler<Data>;

 protected:
    using Func = Handler<Data>;

    Func function_;
    mutable std::recursive_mutex lock_;

 public:
    FuncHandlerBase() { clearMutex(); }
    FuncHandlerBase(Func&& function) : Self() {
        setFunction(std::move(function));
    }
    virtual ~FuncHandlerBase() = default;

    virtual void setFunction(Func&& function) {
        std::lock_guard lock(lock_);
        function_ = std::move(function);
    }

    virtual Mutex getMutex() const = 0;

    virtual void setMutex(const Mutex& lock) const = 0;

    virtual void clearMutex() const = 0;
};

////////////////////////////////////////////////////////////
/// \brief Handler that can call function that will be set.
////////////////////////////////////////////////////////////
template <typename Data>
class SyncFuncHandler : public FuncHandlerBase<Data> {
 protected:
    using Self = SyncFuncHandler<Data>;
    using Base = FuncHandlerBase<Data>;

    using Func = typename Base::Func;
    using Base::lock_;

    mutable Mutex global_lock_;

 public:
    SyncFuncHandler() = default;
    SyncFuncHandler(Func&& function) : Base(std::move(function)) {}
    virtual ~SyncFuncHandler() override = default;

    void setMutex(const Mutex& lock) const { global_lock_ = lock; }

    void clearMutex() const {
        global_lock_ = MutexList::getInstance()->getMutex();
    }

    Mutex getMutex() const { return global_lock_; }

    void call(const Data& data) override {
        std::lock_guard lock(lock_);
        if (Base::function_) {
            std::lock_guard lock(global_lock_);
            Base::function_(data);
        }
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

    using Func = typename Base::Func;
    using Base::lock_;

    ProcessList process_list_;

 public:
    AsyncFuncHandler() = default;

    AsyncFuncHandler(const Mutex& lock) : Base() {
        process_list_.setMutex(lock);
    }

    AsyncFuncHandler(Func&& function) : Base(std::move(function)) {}
    AsyncFuncHandler(Func&& function, const Mutex& lock)
        : Base(std::move(function)) {
        process_list_.setMutex(lock);
    }

    virtual ~AsyncFuncHandler() override = default;

    void setMutex(const Mutex& lock) const { process_list_.setMutex(lock); }

    void clearMutex() const { process_list_.clearMutex(); }

    Mutex getMutex() const { return process_list_.getMutex(); }

    void call(const Data& data) override {
        std::lock_guard lock(lock_);
        if (Base::function_) process_list_.exec(Base::function_, std::ref(data));
    }
};

}  // namespace ec

#endif
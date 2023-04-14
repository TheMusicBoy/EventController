#ifndef _ECL_IMPL_THREAD_HPP_
#define _ECL_IMPL_THREAD_HPP_

#include <ECL/impl/ts_list.hpp>
#include <ECL/impl/singleton.hpp>
#include <condition_variable>
#include <functional>
#include <future>
#include <initializer_list>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

namespace ec {

////////////////////////////////////////////////////////////
/// \brief Object representing a task that can be run in a
/// thread.
////////////////////////////////////////////////////////////
class ThreadTask {
 protected:
    uint32_t priority_;
    bool done_;

    std::function<void()> task_;
    mutable std::condition_variable cv_;
    mutable std::mutex mutex_;

 public:
    ThreadTask();
    ThreadTask(std::function<void()>&& task, size_t priority = 50);
    ThreadTask(const ThreadTask&) = delete;
    ThreadTask(ThreadTask&&)      = delete;
    ~ThreadTask();

    void setPriority(uint32_t priority);
    uint32_t getPriority() const;

    void setTask(std::function<void()>&& task);

    void notify();

    void run();
    void wait();
};

class TaskQueue : protected TsList<ThreadTask*> {
 protected:
    using Self = TaskQueue;
    using Base = TsList<ThreadTask*>;

    std::condition_variable* cv_;

 public:
    TaskQueue();
    TaskQueue(TaskQueue&& other);
    ~TaskQueue();

    void push(ThreadTask* task);
    ThreadTask* pop();
};

class ThreadGroup;

////////////////////////////////////////////////////////////
/// \brief Object that handles thread.
////////////////////////////////////////////////////////////
class ThreadHandler {
 protected:
    ThreadTask* task_;

    ThreadGroup* group_;
    std::thread thread_;
    mutable std::condition_variable cv_;
    mutable std::mutex mutex_;

    bool running_;

    void run();

 public:
    ThreadHandler(ThreadGroup* group);
    ~ThreadHandler();

    void setPriority(uint32_t priority);

    void setTask(ThreadTask* task);
};

struct GroupConfig {
    uint32_t num_threads;
    uint32_t priority;
    uint32_t group_id;

    GroupConfig();
    GroupConfig(uint32_t num_threads, uint32_t group_id,
                uint32_t priority = 50);

    static const GroupConfig EmptyGroup;
    static const GroupConfig DefaultGroup;
};

struct ThreadConfig {
    std::list<GroupConfig> groups;

    ThreadConfig();
    ThreadConfig(std::initializer_list<GroupConfig> groups);
};

class ThreadGroup {
 protected:
    using Container   = std::vector<ThreadHandler*>;
    using ThreadQueue = std::list<ThreadHandler*>;

    Container threads_;
    ThreadQueue queue_;
    TaskQueue tasks_;

    std::condition_variable cv_;
    std::mutex task_mutex_;
    std::mutex thread_mutex_;

 public:
    ThreadGroup(GroupConfig group_config);
    ThreadGroup(ThreadGroup&& other) = delete;
    ThreadGroup(const ThreadGroup&)  = delete;
    ~ThreadGroup();

    void setTasks(ThreadGroup& other);
    void addFreeThread(ThreadHandler* thread);
    void addTask(ThreadTask* task);
    ThreadTask* getTask();
};

////////////////////////////////////////////////////////////
/// \brief ThreadContainer is a container of threads.
////////////////////////////////////////////////////////////
class ThreadContainer {
 protected:
    using Container = std::vector<ThreadGroup*>;

    Container threads_;

 public:
    ThreadContainer(const ThreadConfig& config, size_t number_of_types_);
    ~ThreadContainer();

    void setTaskTypeNumber(size_t task_type_number);
    void addTask(ThreadTask* task, uint32_t task_type);
};

////////////////////////////////////////////////////////////
/// \brief ThreadContainerManager is a manager of threads.
////////////////////////////////////////////////////////////
class ThreadContainerManager : public Singleton<ThreadContainerManager> {
 protected:
    using Container = std::vector<ThreadContainer*>;

    Container containers_;
    size_t task_type_number_;

    ThreadContainerManager();

    friend Singleton<ThreadContainerManager>;

 public:
    ~ThreadContainerManager() override;

    void setTaskTypeNumber(size_t task_type_number_);
    void addConfiguration(const ThreadConfig& config);
    ThreadContainer* getContainer(uint32_t config_id);
};

////////////////////////////////////////////////////////////
/// \brief Object that controls threads.
////////////////////////////////////////////////////////////
class ThreadPool : public Singleton<ThreadPool> {
 protected:
    ThreadContainer* container_;

    ThreadPool();

    friend Singleton<ThreadPool>;

 public:
    ~ThreadPool();

    void setConfig(uint32_t config_id);
    void addTask(ThreadTask* task, uint32_t task_type = 0);
};

////////////////////////////////////////////////////////////
/// \brief Object that represents a single thread.
////////////////////////////////////////////////////////////
class Thread {
 protected:
    ThreadTask task_;

 public:
    template <typename Callable, typename... Args>
    Thread(Callable&& callable, Args&&... args)
        : task_(
              [callable, args...] { callable(std::forward<Args>(args)...); }) {
        ThreadPool::getInstance()->addTask(&task_, 0);
    }

    template <typename Callable, typename... Args>
    Thread(uint32_t group_id, Callable&& callable, Args&&... args)
        : task_(
              [callable, args...] { callable(std::forward<Args>(args)...); }) {
        ThreadPool::getInstance()->addTask(&task_, group_id);
    }

    template <typename Callable, typename... Args>
    Thread(uint32_t group_id, uint32_t priority, Callable&& callable,
           Args&&... args)
        : task_([callable, args...] { callable(std::forward<Args>(args)...); },
                priority) {
        ThreadPool::getInstance()->addTask(&task_, group_id);
    }

    ~Thread();

    void join();
};

////////////////////////////////////////////////////////////
/// \brief Object that represents promise.
////////////////////////////////////////////////////////////
template <typename Result>
class Future : public std::future<Result> {
 protected:
    using Self = Future<Result>;
    using Base = std::future<Result>;

    std::promise<Result>* promise_;

 public:
    Future(std::promise<Result>* promise) : Base(promise->get_future()) {
        promise_ = promise;
    }

    Future(Future&& other) : Base(std::move(other)) {
        promise_       = other.promise_;
        other.promise_ = nullptr;
    }

    Future(const Future<Result>& other) = delete;

    ~Future() {
        if (promise_) delete promise_;
    }
};

////////////////////////////////////////////////////////////
/// \brief DO NOT USE THIS FUNCTION.
/// IT'S NOT FINISHED FOR PRODUCT.
////////////////////////////////////////////////////////////
template <typename Callable, typename... Args>
Future<decltype(std::declval<Callable>()(std::declval<Args>()...))>&& async(
    Callable&& callable, Args&&... args) {
    using Result = decltype(std::declval<Callable>()(std::declval<Args>()...));

    std::promise<Result>* promise = new std::promise<Result>();
    Future<Result> result(promise);

    ThreadTask* task = new ThreadTask([task, promise, callable, args...] {
        promise->set_value(callable(std::forward<Args>(args)...));
        delete task;
    });

    ThreadPool::getInstance()->addTask(task, 0);

    return std::move(result);
}

////////////////////////////////////////////////////////////
/// \brief DO NOT USE THIS FUNCTION.
/// IT'S NOT FINISHED FOR PRODUCT.
////////////////////////////////////////////////////////////
template <typename Callable, typename... Args>
Future<decltype(std::declval<Callable>()(std::declval<Args>()...))>&& async(
    uint32_t group_id, Callable&& callable, Args&&... args) {
    using Result = decltype(std::declval<Callable>()(std::declval<Args>()...));

    std::promise<Result>* promise = new std::promise<Result>();
    Future<Result> result(promise);

    ThreadTask* task = new ThreadTask([task, promise, callable, args...] {
        promise->set_value(callable(std::forward<Args>(args)...));
        delete task;
    });

    ThreadPool::getInstance()->addTask(task, group_id);

    return std::move(result);
}

////////////////////////////////////////////////////////////
/// \brief DO NOT USE THIS FUNCTION.
/// IT'S NOT FINISHED FOR PRODUCT.
////////////////////////////////////////////////////////////
template <typename Callable, typename... Args>
Future<decltype(std::declval<Callable>()(std::declval<Args>()...))>&& async(
    uint32_t group_id, uint32_t priority, Callable&& callable, Args&&... args) {
    using Result = decltype(std::declval<Callable>()(std::declval<Args>()...));

    std::promise<Result>* promise = new std::promise<Result>();
    Future<Result> result(promise);

    ThreadTask* task = new ThreadTask(
        [task, promise, callable, args...] {
            promise->set_value(callable(std::forward<Args>(args)...));
            delete task;
        },
        priority);

    ThreadPool::getInstance()->addTask(task, group_id);

    return std::move(result);
}

}  // namespace ec

#endif
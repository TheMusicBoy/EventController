#include <ECL/impl/thread.hpp>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __linux__
#include <pthread.h>
#endif

#ifdef __APPLE__
#include <pthread.h>
#endif

namespace ec {

////////////////////////////////////////////////////////////
// ThreadTask implementation
////////////////////////////////////////////////////////////

ThreadTask::ThreadTask() : priority_(50), done_(false) {}

ThreadTask::ThreadTask(std::function<void()>&& task, size_t priority)
    : task_(std::move(task)), priority_(priority), done_(false) {}

ThreadTask::~ThreadTask() {}

void ThreadTask::setPriority(uint32_t priority) { priority_ = priority; }

uint32_t ThreadTask::getPriority() const { return priority_; }

void ThreadTask::setTask(std::function<void()>&& task) {
    task_ = std::move(task);
}

void ThreadTask::wait() {
    if (!done_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return done_; });
    }
}

void ThreadTask::run() {
    task_();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        done_ = true;
        cv_.notify_all();
    }
}

////////////////////////////////////////////////////////////
// TaskQueue implementation
////////////////////////////////////////////////////////////

TaskQueue::TaskQueue() {}

TaskQueue::TaskQueue(TaskQueue&& other) : Base(std::move(other)) {}

TaskQueue::~TaskQueue() {}

void TaskQueue::push(ThreadTask* task) { this->Base::push_back(task); }

ThreadTask* TaskQueue::pop() {
    if (!this->Base::empty()) {
        ThreadTask* task = this->Base::front();
        this->Base::pop_front();
        return task;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////
// ThreadHandler implementation
////////////////////////////////////////////////////////////

void ThreadHandler::run() {
    running_ = true;
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        task_ = group_->getTask();

        if (task_ == nullptr) {
            group_->addFreeThread(this);
            cv_.wait(lock, [this] { return task_ != nullptr; });
        }

        task_->run();
        task_ = nullptr;
    }
}

ThreadHandler::ThreadHandler(ThreadGroup* group)
    : thread_(&ThreadHandler::run, std::ref(*this)), group_(group) {}

ThreadHandler::~ThreadHandler() {
    running_ = false;
    thread_.join();
}

void ThreadHandler::setPriority(uint32_t priority) {
#ifdef _WIN32
    int priority_code;
    if (priority < 14)
        priority_code = THREAD_PRIORITY_IDLE;
    else if (priority < 29)
        priority_code = THREAD_PRIORITY_LOWEST;
    else if (priority < 43)
        priority_code = THREAD_PRIORITY_BELOW_NORMAL;
    else if (priority < 57)
        priority_code = THREAD_PRIORITY_NORMAL;
    else if (priority < 71)
        priority_code = THREAD_PRIORITY_ABOVE_NORMAL;
    else if (priority < 86)
        priority_code = THREAD_PRIORITY_HIGHEST;
    else
        priority_code = THREAD_PRIORITY_TIME_CRITICAL;

    SetThreadPriority(thread_.native_handle(), priority_code);
#elif defined(__linux__) || defined(__APPLE__)
    struct sched_param param;
    param.sched_priority = int(priority) / 2.5 - 20;
    pthread_setschedparam(thread_.native_handle(), SCHED_FIFO, &param);
#endif
}

void ThreadHandler::setTask(ThreadTask* task) {
    std::unique_lock<std::mutex> lock(mutex_);
    task_ = task;
    cv_.notify_all();
}

////////////////////////////////////////////////////////////
// GroupConfig implementation
////////////////////////////////////////////////////////////

GroupConfig::GroupConfig() : num_threads(1), group_id(0), priority(50) {}

GroupConfig::GroupConfig(uint32_t num_threads, uint32_t group_id,
                         uint32_t priority)
    : num_threads(num_threads), group_id(group_id), priority(priority) {}

const GroupConfig GroupConfig::EmptyGroup(0, 0, 0);
const GroupConfig GroupConfig::DefaultGroup(1, 0);

////////////////////////////////////////////////////////////
// ThreadGroup implementation
////////////////////////////////////////////////////////////

ThreadGroup::ThreadGroup(GroupConfig group_config) {
    for (uint32_t i = 0; i < group_config.num_threads; ++i) {
        threads_.push_back(new ThreadHandler(this));
        threads_[i]->setPriority(group_config.priority);
    }
}

ThreadGroup::~ThreadGroup() {
    for (auto& thread : threads_) delete thread;
}

void ThreadGroup::setTasks(ThreadGroup& other) {
    std::lock_guard<std::mutex> lock(task_mutex_);
    this->queue_ = std::move(other.queue_);
}

void ThreadGroup::addTask(ThreadTask* task) {
    std::lock_guard<std::mutex> lock(task_mutex_);
    if (queue_.empty())
        tasks_.push(task);
    else {
        queue_.front()->setTask(task);
        queue_.pop_front();
    }
}

ThreadTask* ThreadGroup::getTask() {
    std::lock_guard<std::mutex> lock(task_mutex_);
    return tasks_.pop();
}

void ThreadGroup::addFreeThread(ThreadHandler* handler) {
    std::lock_guard<std::mutex> lock(thread_mutex_);
    queue_.push_back(handler);
}

////////////////////////////////////////////////////////////
// ThreadConfig implementation
////////////////////////////////////////////////////////////

ThreadConfig::ThreadConfig() = default;

ThreadConfig::ThreadConfig(std::initializer_list<GroupConfig> groups)
    : groups(groups) {}

////////////////////////////////////////////////////////////
// ThreadContainer implementation
////////////////////////////////////////////////////////////

ThreadContainer::ThreadContainer(const ThreadConfig& config,
                                 size_t number_of_types_) {
    threads_.resize(number_of_types_, nullptr);

    for (auto& el : config.groups) threads_[el.group_id] = new ThreadGroup(el);

    for (auto& el : threads_)
        if (el == nullptr) el = new ThreadGroup(GroupConfig::EmptyGroup);
}

ThreadContainer::~ThreadContainer() {
    for (auto& el : threads_) delete el;
}

void ThreadContainer::setTaskTypeNumber(size_t number_of_types) {
    threads_.resize(number_of_types, nullptr);

    for (auto& el : threads_)
        if (el == nullptr) el = new ThreadGroup(GroupConfig::EmptyGroup);
}

void ThreadContainer::addTask(ThreadTask* task, uint32_t task_type) {
    threads_[task_type]->addTask(task);
}

////////////////////////////////////////////////////////////
// ThreadContainerManager implementation
////////////////////////////////////////////////////////////

ThreadContainerManager::ThreadContainerManager() {
    setTaskTypeNumber(1);
    addConfiguration({{std::thread::hardware_concurrency(), 0}});
};

ThreadContainerManager::~ThreadContainerManager() = default;

void ThreadContainerManager::setTaskTypeNumber(size_t number_of_types) {
    task_type_number_ = number_of_types;
    for (auto& el : containers_) el->setTaskTypeNumber(task_type_number_);
}

void ThreadContainerManager::addConfiguration(const ThreadConfig& config) {
    containers_.push_back(new ThreadContainer(config, task_type_number_));
}

ThreadContainer* ThreadContainerManager::getContainer(uint32_t config_id) {
    return containers_.at(config_id);
}

////////////////////////////////////////////////////////////
// ThreadPool implementation
////////////////////////////////////////////////////////////

ThreadPool::ThreadPool() { setConfig(0); }

ThreadPool::~ThreadPool() = default;

void ThreadPool::addTask(ThreadTask* task, uint32_t task_type) {
    container_->addTask(task, task_type);
}

void ThreadPool::setConfig(uint32_t config_id) {
    container_ = ThreadContainerManager::getInstance()->getContainer(config_id);
}

////////////////////////////////////////////////////////////
// Thread implementation
////////////////////////////////////////////////////////////

Thread::~Thread() { task_.wait(); }

void Thread::join() { task_.wait(); }

}  // namespace ec
#ifndef _TMBEL_PROCESS_LIST_HPP_
#define _TMBEL_PROCESS_LIST_HPP_

#include <TMBEL/lock_handler.hpp>
#include <TMBEL/multithread_list.hpp>
#include <thread>

namespace ec {

class ProcessList {
 protected:
    using Container = MtListBase<std::thread>;

    Container resource_;
    mutable std::recursive_mutex lock_;
    mutable Mutex global_lock_;

 public:
    ProcessList();
    ~ProcessList();

    void setMutex(const Mutex& lock) const;
    Mutex getMutex() const;
    void clearMutex() const;

    template<typename Callable, typename... Args>
    void exec(Callable&& callable, Args&&... args) {
        std::lock_guard lock(lock_);
        resource_.emplace_back([this, callable, args...](){
            global_lock_.lock();
            callable(std::move<Args>(args)...);
            global_lock_.unlock();
        });
    }

    void clear();
};

}  // namespace ec

#endif
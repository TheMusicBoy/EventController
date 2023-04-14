#ifndef _ECL_PROCESS_LIST_HPP_
#define _ECL_PROCESS_LIST_HPP_

#include <ECL/impl/lock_handler.hpp>
#include <ECL/impl/ts_list.hpp>
#include <ECL/impl/thread.hpp>

namespace ec {

class ProcessList {
 protected:
    using Container = TsList<Thread>;

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
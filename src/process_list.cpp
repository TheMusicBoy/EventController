#include <ECL/process_list.hpp>

namespace ec {

////////////////////////////////////////////////////////////
// ProcessList implementation
////////////////////////////////////////////////////////////

ProcessList::ProcessList() = default;
ProcessList::~ProcessList() {
    std::lock_guard lock(lock_);
    clear();
}

void ProcessList::setMutex(const Mutex& new_lock) const {
    std::lock_guard lock(lock_);
    global_lock_ = new_lock;
}

void ProcessList::clearMutex() const {
    std::lock_guard lock(lock_);
    global_lock_ = MutexList::getInstance()->getMutex();
}

Mutex ProcessList::getMutex() const { return global_lock_; }

void ProcessList::clear() {
    std::lock_guard lock(lock_);
    resource_.map([](Thread& el) { el.join(); });
    resource_.clear();
}

}  // namespace ec

#include <TMBEL/lock_handler.hpp>

namespace ec {

////////////////////////////////////////////////////////////
// MutexObjectBase implementation
////////////////////////////////////////////////////////////

MutexObjectBase::MutexObjectBase() : ref_counter_(0) {}

MutexObjectBase::~MutexObjectBase() {}

std::recursive_mutex& MutexObjectBase::get() { return lock_; }

void MutexObjectBase::increase() { ++ref_counter_; }

void MutexObjectBase::decrease() {
    --ref_counter_;
    if (ref_counter_ == 0) this->~MutexObjectBase();
}

////////////////////////////////////////////////////////////
// Mutex
////////////////////////////////////////////////////////////

Mutex::Mutex(MutexObjectBase* pointer) : reference_(pointer) {
    reference_->increase();
}

Mutex::Mutex(const Mutex& other) : reference_(other.reference_) {
    reference_->increase();
}

Mutex::Mutex(Mutex&& other) : reference_(other.reference_) {
    other.reference_ = nullptr;
}
Mutex::~Mutex() {
    reference_->decrease();
}

void Mutex::lock() {
    reference_->get().lock();
}

void Mutex::unlock() {
    reference_->get().unlock();
}

////////////////////////////////////////////////////////////
// MutexObject implementation
////////////////////////////////////////////////////////////

MutexObject::MutexObject() = default;

MutexObject::~MutexObject() = default;

Mutex MutexObject::createRef() {
    return Mutex(this);
}

////////////////////////////////////////////////////////////
// MutexList implementation
////////////////////////////////////////////////////////////

MutexList::MutexList() = default;

Mutex MutexList::getMutex() {
    auto new_mutex = new MutexObject();
    resource_.push_back(new_mutex);
    return new_mutex->createRef();
}


}  // namespace ec
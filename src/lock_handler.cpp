#include <ECL/lock_handler.hpp>

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

void Mutex::decrease_() {
    if (reference_ != nullptr)
        ;
    reference_->decrease();
}

void Mutex::increase_() {
    if (reference_ != nullptr) reference_->increase();
}

Mutex::Mutex() : reference_(nullptr) {}

Mutex::Mutex(MutexObjectBase* pointer) : reference_(pointer) { increase_(); }

Mutex::Mutex(const Mutex& other) : reference_(other.reference_) { increase_(); }

Mutex::Mutex(Mutex&& other) : reference_(other.reference_) {
    other.reference_ = nullptr;
}

Mutex::~Mutex() { decrease_(); }

Mutex& Mutex::operator=(const Mutex& other) {
    if (this != &other) {
        decrease_();
        reference_ = other.reference_;
        increase_();
    }
    return *this;
}

Mutex& Mutex::operator=(Mutex&& other) {
    if (this != &other) {
        decrease_();
        reference_ = other.reference_;
        other.reference_ = nullptr;
    }
    return *this;
}

void Mutex::lock() {
    if (reference_ != nullptr) reference_->get().lock();
}

void Mutex::unlock() {
    if (reference_ != nullptr) reference_->get().unlock();
}

////////////////////////////////////////////////////////////
// MutexObject implementation
////////////////////////////////////////////////////////////

MutexObject::MutexObject() = default;

MutexObject::~MutexObject() = default;

Mutex MutexObject::createRef() { return Mutex(this); }

////////////////////////////////////////////////////////////
// MutexList implementation
////////////////////////////////////////////////////////////

MutexList::MutexList() = default;

Mutex MutexList::getMutex() {
    auto new_mutex = new MutexObject();
    sub_list_.push_back(new_mutex);
    return new_mutex->createRef();
}

}  // namespace ec
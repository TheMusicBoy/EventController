#include <TMBEL/utils.hpp>

namespace ec {

////////////////////////////////////////////////////////////
// Singleton implementation
////////////////////////////////////////////////////////////

SingletonList::~SingletonList() {
    std::lock_guard lock(lock_);
    for (auto el : resource_)
        delete el;
    resource_.clear();
}

SingletonList* SingletonList::getInstance() {
    if (instance_ == nullptr) instance_ = new SingletonList();
    return instance_;
}

std::list<SingletonBase*>::iterator SingletonList::add(SingletonBase* object) {
    std::lock_guard lock(lock_);
    resource_.push_back(object);
    return std::prev(resource_.end());
}

void SingletonList::remove(Position position) {
    std::lock_guard lock(lock_);
    resource_.erase(position);
}

void clearResource() {
    delete SingletonList::getInstance();
}

SingletonBase::SingletonBase() {
    position_ = SingletonList::getInstance()->add(this);
}

void SingletonBase::detachSingleton() {
    SingletonList::getInstance()->remove(position_);
}

////////////////////////////////////////////////////////////
// UniqueContainer implementation
////////////////////////////////////////////////////////////

UniqueContainer::UniqueContainer(UniqueContainer&& other) {
    other.lock_.lock();
    this->lock_.lock();

    resource_ = std::move(other.resource_);

    this->lock_.unlock();
    other.lock_.unlock();
}

UniqueContainer::~UniqueContainer() {
    std::lock_guard lock(lock_);
    for (auto el : resource_) delete el;
    resource_.clear();
}

std::list<HandlerBase*>::iterator UniqueContainer::push(HandlerBase* handler) {
    std::lock_guard lock(lock_);
    resource_.push_back(handler);
    return std::prev(resource_.end());
}

HandlerBase* UniqueContainer::pop(Position position) {
    std::lock_guard lock(lock_);
    HandlerBase* result = *position;
    resource_.erase(position);
    return result;
}

void UniqueContainer::del(Position position) {
    std::lock_guard lock(lock_);
    delete *position;
    resource_.erase(position);
}

}  // namespace ec

#include <TMBEL/handler.hpp>

namespace el {

////////////////////////////////////////////////////////////
// HandlerBase implementation
////////////////////////////////////////////////////////////

HandlerBase::HandlerBase() : container_(nullptr) {}

HandlerBase::HandlerBase(Container* container) : container_(container) {
    position_ = container_->push(this);
}

HandlerBase::HandlerBase(Container* container, Position position)
    : container_(container) {
    position_ = container_->insert(position, this);
}

HandlerBase::~HandlerBase() { detach(); }

void HandlerBase::attach(Container* container) {
    detach();
    container_ = container;
    position_  = container_->push(this);
}

void HandlerBase::attach(Container* container, Position position) {
    container_ = container;
    position_ = container_->insert(position, this);
}

void HandlerBase::detach() {
    if (container_ != nullptr) {
        container_->pop(position_);
        container_ = nullptr;
    }
}

void HandlerBase::remove() {}

////////////////////////////////////////////////////////////
// HandlerContainerBase implementation
////////////////////////////////////////////////////////////

HandlerListBase::HandlerListBase() : resource_() {}

HandlerListBase::HandlerListBase(HandlerListBase&& other) {
    other.lock_.lock();
    this->lock_.lock();

    resource_ = std::move(other.resource_);

    this->lock_.unlock();
    other.lock_.unlock();
}

HandlerListBase::~HandlerListBase() { clear(); }

std::list<HandlerBase*>::iterator HandlerListBase::push(
    HandlerBase* handler) {
    std::lock_guard lock(lock_);
    resource_.push_back(handler);
    return std::prev(resource_.end());
}

std::list<HandlerBase*>::iterator HandlerListBase::insert(
    HandlerPos position, HandlerBase* handler) {
    std::lock_guard lock(lock_);
    return resource_.insert(position, handler);
}

HandlerBase* HandlerListBase::pop(HandlerPos position) {
    HandlerBase* pointer = *position;

    std::lock_guard lock(lock_);
    resource_.erase(position);

    return pointer;
}

void HandlerListBase::clear() {
    std::lock_guard lock(lock_);
    for (auto el : resource_)
        el->detach();
}

size_t HandlerListBase::size() { return resource_.size(); }

bool HandlerListBase::empty() { return resource_.empty(); }


}  // namespace ev
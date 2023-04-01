#include <TMBEL/utils.hpp>

namespace ec {

////////////////////////////////////////////////////////////
// UniqueContainer implementation
////////////////////////////////////////////////////////////

UniqueContainer::UniqueContainer() = default;

UniqueContainer::UniqueContainer(UniqueContainer&& other)
    : Base(std::move(other)) {}

UniqueContainer::~UniqueContainer() {
    std::lock_guard lock(Base::lock_);
    for (auto el : resource_) delete el;
}

inline UniqueContainer::Position UniqueContainer::push(
    HandlerBase* handler) {
    return Base::push_back(handler);
}

HandlerBase* UniqueContainer::pop(Position position) {
    HandlerBase* pointer = *position;
    Base::erase(position);
    return pointer;
}

void UniqueContainer::del(Position position) {
    std::lock_guard lock(Base::lock_);
    delete *position;
    Base::erase(position);
}

}  // namespace ec

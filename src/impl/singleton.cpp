#include <ECL/impl/singleton.hpp>

namespace ec {


////////////////////////////////////////////////////////////
// Singleton implementation
////////////////////////////////////////////////////////////

SingletonList::SingletonList() = default;

SingletonList::~SingletonList() {
    this->map([](SingletonBase* el) { delete el; });
}

SingletonList* SingletonList::getInstance() {
    if (instance_ == nullptr) instance_ = new SingletonList();
    return instance_;
}

void clearResource() { delete SingletonList::getInstance(); }

SingletonBase::SingletonBase() {
    position_ = SingletonList::getInstance()->push_back(this);
}

SingletonBase::~SingletonBase() = default;

void SingletonBase::detachSingleton() {
    SingletonList::getInstance()->erase(position_);
}

}  // namespace ec

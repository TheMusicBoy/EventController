#include <TMBEL/multithread_list.hpp>

namespace ec {

////////////////////////////////////////////////////////////
// SubObjectBase implementation
////////////////////////////////////////////////////////////

SubObjectBase::SubObjectBase() : container_(nullptr) {}

SubObjectBase::SubObjectBase(Container* container) { attach(container); }

SubObjectBase::SubObjectBase(Position position, Container* container) {
    attach(position, container);
}

SubObjectBase::~SubObjectBase() { detach(); }

SubObjectBase::Position SubObjectBase::attach(Container* container) {
    container_       = container;
    return position_ = container_->push_back(this);
}

SubObjectBase::Position SubObjectBase::attach(Position position,
                                              Container* container) {
    container_       = container;
    return position_ = container_->insert(position, this);
}

void SubObjectBase::detach() {
    if (container_ != nullptr) {
        container_->erase(position_);
        container_ = nullptr;
    }
}

bool SubObjectBase::isAttached() const { return container_ != nullptr; }

////////////////////////////////////////////////////////////
// ObsObjectBase implementation
////////////////////////////////////////////////////////////

ObsObjectBase::ObsObjectBase() = default;

ObsObjectBase::~ObsObjectBase() = default;

ObsObjectBase::Position ObsObjectBase::attach(Object* object) {
    return object->attach(&resource_);
}

ObsObjectBase::Position ObsObjectBase::attach(Position position, Object* object) {
    return object->attach(position, &resource_);
}

}  // namespace ec
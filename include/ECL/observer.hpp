#ifndef _ECL_OBSERVER_BASE_HPP_
#define _ECL_OBSERVER_BASE_HPP_

#include <ECL/ts_list.hpp>

namespace ec {
    
template <typename SubType>
class SubObjectBase {
 protected:
    using Self = SubObjectBase;

    using Container = TsList<SubType*>;

 public:
    using Position = typename Container::Position;

 protected:
    Position position_;
    Container* container_;

 public:
    SubObjectBase() : container_(nullptr) {}
    SubObjectBase(Container* container) { attachTo(container); }
    SubObjectBase(Position position, Container* container) {
        attachTo(position, container);
    }
    virtual ~SubObjectBase() { detach(); }

    Position attachTo(Container* container) {
        detach();
        container_       = container;
        return position_ = container_->push_back(static_cast<SubType*>(this));
    }
    Position attachTo(Position position, Container* container) {
        detach();
        container_ = container;
        return position_ =
                   container_->insert(position, static_cast<SubType*>(this));
    }
    void detach() {
        if (container_ != nullptr) {
            container_->erase(position_);
            container_ = nullptr;
        }
    }

    bool isAttached() const { return container_ != nullptr; }
};

template <typename SubType>
class ObsObjectBase {
 protected:
    static_assert(std::is_base_of<SubObjectBase<SubType>, SubType>::value,
                  "Subscriber object must be inherited by ec::SubObjectBase.");

    using Container = TsList<SubType*>;
    using Object    = SubType;

    Container sub_list_;

 public:
    using Position = typename Container::Position;

    ObsObjectBase()          = default;
    virtual ~ObsObjectBase() = default;

    inline void map(std::function<void(SubType*)> func) { sub_list_.map(func); }

    inline void map(std::function<void(const SubType*)> func) const {
        sub_list_.map(func);
    }

    inline Position attach(Object* object) {
        return object->attachTo(&sub_list_);
    }

    inline Position attach(Position position, Object* object) {
        return object->attachTo(position, &sub_list_);
    }
};
    
} // namespace ec

#endif
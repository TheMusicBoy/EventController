#ifndef _TMBEL_MULTITHREAD_LIST_HPP_
#define _TMBEL_MULTITHREAD_LIST_HPP_

#include <functional>
#include <list>
#include <mutex>
#include <type_traits>

namespace ec {

template <typename Ty>
class MtListBase {
 protected:
    using Self      = MtListBase;
    using Container = std::list<Ty>;

    mutable std::recursive_mutex lock_;
    Container resource_;

 public:
    using Position        = typename Container::iterator;
    using value_type      = Ty;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using distance_type   = ptrdiff_t;

    MtListBase() = default;
    MtListBase(const Self& other) { *this = other; }
    MtListBase(Self&& other) { *this = std::move(other); }
    ~MtListBase() {
        std::lock_guard lock(lock_);
        clear();
    }

    Self& operator=(const Self& other) {
        other.lock_.lock();
        this->lock_.lock();

        resource_ = other.resource_;

        this->lock_.unlock();
        other.lock_.unlock();

        return *this;
    }

    Self& operator=(Self&& other) {
        other.lock_.lock();
        this->lock_.lock();

        resource_ = std::move(other.resource_);

        this->lock_.unlock();
        other.lock_.unlock();

        return *this;
    }

    Position push_back(const value_type& object) {
        std::lock_guard lock(lock_);
        resource_.push_back(object);
        return std::prev(resource_.end());
    }

    Position push_front(const value_type& object) {
        std::lock_guard lock(lock_);
        resource_.push_front(object);
        return resource_.begin();
    }

    Position insert(Position position, const value_type& object) {
        std::lock_guard lock(lock_);
        return resource_.insert(position, object);
    }

    template <typename... Args>
    Position emplace(Position position, Args&&... args) {
        std::lock_guard lock(lock_);
        return resource_.emplace(position, std::move(args...));
    }

    template <typename... Args>
    Position emplace_back(Args&&... args) {
        std::lock_guard lock(lock_);
        resource_.emplace_back(std::move(args...));
        return std::prev(resource_.end());
    }

    template <typename... Args>
    Position emplace_front(Args&&... args) {
        std::lock_guard lock(lock_);
        resource_.emplace_front(std::move(args...));
        return resource_.begin();
    }

    void erase(Position position) {
        std::lock_guard lock(lock_);
        resource_.erase(position);
    }

    void erase(Position begin, Position end) {
        std::lock_guard lock(lock_);
        resource_.erase(begin, end);
    }

    void clear() {
        std::lock_guard lock(lock_);
        resource_.clear();
    }

    void splice(Position position, Self& other) {
        other.lock_.lock();
        this->lock_.lock();

        resource_.splice(position, other);

        this->lock_.unlock();
        other.lock_.unlock();
    }

    void splice(Position position, Self&& other) {
        other.lock_.lock();
        this->lock_.lock();

        resource_.splice(position, std::move(other));

        this->lock_.unlock();
        other.lock_.unlock();
    }

    size_t size() const {
        std::lock_guard lock(lock_);
        return resource_.size();
    }

    bool empty() const {
        std::lock_guard lock(lock_);
        return resource_.empty();
    }

    void map(std::function<Ty> func) {
        std::lock_guard lock(lock_);
        for (auto& el : resource_) func(el);
    }

    void map(std::function<void(Ty&)> func) {
        std::lock_guard lock(lock_);
        for (auto& el : resource_) func(el);
    }

    void map(std::function<void(const Ty&)> func) const {
        std::lock_guard lock(lock_);
        for (auto& el : resource_) func(el);
    }
};

template <typename SubType>
class SubObjectBase {
 protected:
    using Self = SubObjectBase;

    using Container = MtListBase<SubType*>;

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
        container_       = container;
        return position_ = container_->insert(position, static_cast<SubType*>(this));
    }
    void detach() {
        if (container_ != nullptr) {
            container_->erase(position_);
            container_ = nullptr;
        }
    }

    bool isAttached() const { return container_ != nullptr; }
};

template <
    typename SubType>
class ObsObjectBase {
 protected:
    using Container = MtListBase<SubType*>;
    using Object    = SubType;

    Container sub_list_;

 public:
    using Position = typename Container::Position;

    ObsObjectBase()  = default;
    virtual ~ObsObjectBase() = default;

    inline Position attach(Object* object) {
        return object->attachTo(&sub_list_);
    }

    inline Position attach(Position position, Object* object) {
        return object->attachTo(position, &sub_list_);
    }
};

}  // namespace ec

#endif
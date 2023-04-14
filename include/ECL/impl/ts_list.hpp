#ifndef _ECL_THREAD_SAFE_LIST_HPP_
#define _ECL_THREAD_SAFE_LIST_HPP_

#include <functional>
#include <list>
#include <mutex>
#include <type_traits>

namespace ec {

template <typename Ty>
class TsList {
 protected:
    using Self      = TsList;
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

    TsList() = default;
    TsList(const Self& other) { *this = other; }
    TsList(Self&& other) { *this = std::move(other); }
    ~TsList() {
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

    void pop_back() {
        std::lock_guard lock(lock_);
        resource_.pop_back();
    }

    void pop_front() {
        std::lock_guard lock(lock_);
        resource_.pop_front();
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

    value_type& front() {
        std::lock_guard lock(lock_);
        return resource_.front();
    }

    value_type& back() {
        std::lock_guard lock(lock_);
        return resource_.back();
    }

    const value_type& front() const {
        std::lock_guard lock(lock_);
        return resource_.front();
    }

    const value_type& back() const {
        std::lock_guard lock(lock_);
        return resource_.back();
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

    void map(std::function<void(Ty&)> func) {
        std::lock_guard lock(lock_);
        for (auto& el : resource_) func(el);
    }

    void map(std::function<void(const Ty&)> func) const {
        std::lock_guard lock(lock_);
        for (auto& el : resource_) func(el);
    }
};

}  // namespace ec

#endif
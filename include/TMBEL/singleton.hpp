#ifndef _TMBEL_SINGLETON_HPP_
#define _TMBEL_SINGLETON_HPP_

#include <TMBEL/multithread_list.hpp>

namespace ec {
    
////////////////////////////////////////////////////////////
/// \brief Class implement singleton pattern
////////////////////////////////////////////////////////////

class SingletonBase;

class SingletonList : public MtListBase<SingletonBase*> {
 protected:
    using Self = SingletonList;
    using Base = MtListBase<SingletonBase*>;

    SingletonList();

    static inline SingletonList* instance_;

 public:

    ~SingletonList();

    static inline SingletonList* getInstance();
};

void clearResource();

class SingletonBase {
 private:
    using Position = std::list<SingletonBase*>::iterator;

    Position position_;

 public:
    SingletonBase();
    virtual ~SingletonBase();

    void detachSingleton();
};

template <class Class>
class Singleton : protected SingletonBase {
 private:
    using Self = Singleton<Class>;

    static inline Class* instance_;

 protected:
    Singleton() = default;
    ~Singleton() override { delete instance_; }

 public:
    static inline Class* getInstance() {
        if (instance_ == nullptr) instance_ = new Class();
        return instance_;
    }
};
    
} // namespace ec

#endif
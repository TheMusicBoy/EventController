#ifndef _TMBEL_LOCK_HANDLER_HPP_
#define _TMBEL_LOCK_HANDLER_HPP_

#include <TMBEL/multithread_list.hpp>
#include <TMBEL/utils.hpp>
#include <mutex>

namespace ec {

class MutexObjectBase {
 protected:
    size_t ref_counter_;
    std::recursive_mutex lock_;

 public:
    MutexObjectBase();
    virtual ~MutexObjectBase();

    std::recursive_mutex& get();
    void increase();
    void decrease();

};

class Mutex {
 protected:
    MutexObjectBase* reference_;

 public:
    Mutex(MutexObjectBase* pointer);
    Mutex(const Mutex& other);
    Mutex(Mutex&& other);
    ~Mutex();

    void lock();
    void unlock();

};

class MutexObject : protected MutexObjectBase, public SubObjectBase {
 protected:
    using Self = MutexObject;
    using Base = MutexObjectBase;
    

 public:
    MutexObject();
    ~MutexObject() override;
    Mutex createRef();
};

class MutexList : protected ObsObjectBase, public Singleton<MutexList> {
 protected:
    using Self     = MutexList;
    using Base     = ObsObjectBase;
    using Position = typename Base::Position;

    MutexList();

    friend Singleton<MutexList>;

 public:
    Mutex getMutex();

};

}  // namespace ec

#endif
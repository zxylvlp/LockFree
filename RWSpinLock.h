#include "atomic.h"
class RWSpinLock {
    struct Atomic {
        union {
            uint64_t v_;
            struct {
                uint64_t r_cnt_ :62;
                uint64_t w_pending_ :1;
                uint64_t w_flag_ :1;
            };
        };
        Atomic() :
                v_(0) {
        }
    };
public:
    void rLock() {
        while (true) {
            Atomic old_v = ATOMIC_LOAD(&atomic_);
            Atomic new_v = old_v;
            new_v.r_cnt_++;
            if (old_v.w_flag_
                    == 0&& old_v.w_pending_ == 0 && CAS(&atomic_.v_, old_v.v_, new_v.v_)) {
                break;
            }
            PAUSE();
        }
    }

    void rUnLock() {
        ADD_AND_FETCH(&atomic_.v_, -1);
    }

    void wLock() {
        while (true) {
            Atomic old_v = ATOMIC_LOAD(&atomic_);
            Atomic new_v = old_v;
            bool pending = false;
            if (old_v.r_cnt_ != 0 || old_v.w_flag_ != 0) {
                new_v.w_pending_ = 1;
                pending = true;
            } else {
                new_v.w_flag_ = 1;
                new_v.w_pending_ = 0;
            }
            if (CAS(&atomic_.v_, old_v.v_, new_v.v_)) {
                if (!pending) {
                    break;
                }
            }
            PAUSE();
        }
    }

    void wUnLock() {
        COMPILER_BARRIER();
        atomic_.w_flag_ = 0;
    }
private:
    Atomic atomic_;
};


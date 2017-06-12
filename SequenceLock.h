#include <mutex>
#include <stdint.h>
#include "atomic.h"
class SequenceLock {
private:
    uint64_t seq_;
    std::mutex mu_;
public:
    SequenceLock() :
            seq_(0) {
    }
    void wLock() {
        mu_.lock();
        seq_++;
        COMPILER_BARRIER();
    }
    void wUnlock() {
        COMPILER_BARRIER();
        seq_++;
        mu_.unlock();
    }
    uint64_t rBegin() {
        uint64_t seq = 0;
        while (true) {
            seq = ATOMIC_LOAD(&seq_);
            if (!(seq & 1)) {
                break;
            }
        }
        return seq;
    }
    bool rSuccess(uint64_t seq) {
        MEM_BARRIER();
        return ATOMIC_LOAD(&seq_) == seq;
    }
};

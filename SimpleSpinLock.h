/*
 * SimpleSpinLock.h
 *
 *  Created on: 2017年6月12日
 *      Author: zxy
 */

#ifndef SIMPLESPINLOCK_H_
#define SIMPLESPINLOCK_H_
#include <stdint.h>
#include <assert.h>
#include "atomic.h"
class SimpleSpinLock {
public:
    SimpleSpinLock() :
            atomic_(0) {
    }
    bool try_lock() {
        return ATOMIC_LOAD(&atomic_) == 0 && CAS(&atomic_, 0, 1);

    }
    void lock() {
        while (!try_lock()) {
            PAUSE();
        }
    }
    void unlock() {
        atomic_ = 0;
    }
private:
    int8_t atomic_;
};

#endif /* SIMPLESPINLOCK_H_ */

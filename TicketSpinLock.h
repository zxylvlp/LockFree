/*
 * TicketSpinLock.h
 *
 *  Created on: 2017年6月12日
 *      Author: zxy
 */

#ifndef TICKETSPINLOCK_H_
#define TICKETSPINLOCK_H_
#include <stdint.h>
#include "atomic.h"
class TicketSpinLock {
public:
    void lock() {
        uint64_t my_id = FETCH_AND_ADD(&next_id_, 1);
        while (ATOMIC_LOAD(&service_id_) != my_id) {
            PAUSE();
        }
    }
    void unlock() {
        ADD_AND_FETCH(&service_id_, 1);
    }
private:
    uint64_t next_id_;
    uint64_t service_id_;
};





#endif /* TICKETSPINLOCK_H_ */

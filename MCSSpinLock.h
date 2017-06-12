/*
 * MCSSpinLock.h
 *
 *  Created on: 2017年6月12日
 *      Author: zxy
 */

#ifndef MCSSPINLOCK_H_
#define MCSSPINLOCK_H_
#include "atomic.h"
struct MCSNode {
    bool waiting_;
    MCSNode *next_;
};
class MCSSpinLock {
public:
    void lock() {
        MCSNode *tls_node = get_tls_node();
        tls_node->next_ = nullptr;
        MCSNode *prev_gnode = ATOMIC_EXCHANGE(&gnode_, tls_node);
        if (prev_gnode==nullptr) {
            return;
        }
        tls_node->waiting_ = true;
        COMPILER_BARRIER();
        prev_gnode->next_ = tls_node;
        while (ATOMIC_LOAD(&tls_node->waiting_)) {
            PAUSE();
        }
        MEM_BARRIER();
    }
    void unlock() {
        MEM_BARRIER();
        MCSNode *tls_node = get_tls_node();
        if (tls_node->next_==nullptr) {
            if (CAS(&gnode_, tls_node, nullptr)) {
                return;
            }
            while (!ATOMIC_LOAD(&tls_node->next_)) {
                PAUSE();
            }
        }
        tls_node->next_->waiting_ = false;
    }
private:
    MCSNode* gnode_ = nullptr;
    MCSNode *get_tls_node() {
        static __thread MCSNode tls_node;
        return &tls_node;
    }
};
#endif /* MCSSPINLOCK_H_ */

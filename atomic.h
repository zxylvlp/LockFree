/*
 * atomic.h
 *
 *  Created on: 2017年6月12日
 *      Author: zxy
 */

#ifndef ATOMIC_H_
#define ATOMIC_H_
#include <stdint.h>

#define COMPILER_BARRIER() __asm__ __volatile__("" : : : "memory")
#define MEM_BARRIER() __sync_synchronize()

#define CAS(address,oldValue,newValue) __sync_bool_compare_and_swap(address,oldValue,newValue)
#define CAS_RET_VAL(address,oldValue,newValue) __sync_val_compare_and_swap(address,oldValue,newValue)
#define ADD_AND_FETCH(address,offset) __sync_add_and_fetch(address,offset)
#define FETCH_AND_ADD(address,offset) __sync_fetch_and_add(address,offset)

#define ATOMIC_LOAD(x) ({COMPILER_BARRIER(); *(x);})
#define ATOMIC_STORE(x, v) ({COMPILER_BARRIER(); *(x) = v; MEM_BARRIER(); })
#define ATOMIC_EXCHANGE(address, val) __atomic_exchange_n(address, val, __ATOMIC_SEQ_CST)
#define PAUSE() __asm__ __volatile__("pause\n" : : : "memory")
#define CACHE_ALIGN_SIZE 64
#define CACHE_ALIGNED __attribute__((aligned(CACHE_ALIGN_SIZE)))



#endif /* ATOMIC_H_ */

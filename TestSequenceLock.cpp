/*
 * TestSequenceLock.cpp
 *
 *  Created on: 2017年6月28日
 *      Author: zxy
 */
#include "SequenceLock.h"
#include "MCSSpinLock.h"
#include <thread>
#include <vector>
#include <iostream>
int main() {
    SequenceLock sl;
    MCSSpinLock mcssl;
    std::vector<std::thread> threads;
    int num = 10;
    for (int i=0;i<10;i++) {
        std::thread t([i, &sl, &num, &mcssl]{
            while (true) {
                bool succ = false;
                int val = -1;
                while(!succ) {
                    uint64_t seq = sl.rBegin();
                    val = num;
                    succ = sl.rSuccess(seq);
                }
                mcssl.lock();
                std::cout << "read: " << val << std::endl;
                mcssl.unlock();
            }
        });
        threads.emplace_back(std::move(t));
    }
    for (int i=0; ;i++) {
        sl.wLock();
        num = i;
        sl.wUnlock();
        mcssl.lock();
        std::cout << "write: " << i << std::endl;
        mcssl.unlock();
    }
}





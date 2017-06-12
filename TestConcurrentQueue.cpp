/*
 * TestConcurrentQueue.cpp
 *
 *  Created on: 2017年7月2日
 *      Author: zxy
 */
#include "concurrent_queue.h"
#include "SimpleSpinLock.h"
#include <thread>
#include <vector>
#include <iostream>
int main() {
    ConcurrentQueue<uint64_t> cq;
    SimpleSpinLock ssl;
    std::vector<std::thread> threads;
    for (int i=0;i<10;i++) {
        std::thread t([i, &cq, &ssl]{
            if (i & 1) {
                uint64_t j = 0;
                while (true) {
                    cq.enqueue(j, i);
                    //ssl.lock();
                    //std::cout << "e: " << j << " tid: " << i <<std::endl;
                    //ssl.unlock();
                    j++;
                }
            } else {
                while (true) {
                    uint64_t j = cq.dequeue(i);
                    //ssl.lock();
                    //std::cout << "d: " << j << " tid: " << i << std::endl;
                    //ssl.unlock();
                }
            }
        });
        threads.emplace_back(std::move(t));
    }
    while (true);
}

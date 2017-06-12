#include "concurrent_queue_with_version.h"
#include "SimpleSpinLock.h"
#include <thread>
#include <vector>
#include <iostream>
int main() {
    ConcurrentQueueWithVersion<uint64_t> cq;
    SimpleSpinLock ssl;
    std::vector<std::thread> threads;
    for (uint16_t i=0;i<10;i++) {
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

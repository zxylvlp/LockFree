/*
 * TestSequenceLock.cpp
 *
 *  Created on: 2017年6月28日
 *      Author: zxy
 */
#include "RWSpinLock.h"
#include "MCSSpinLock.h"
#include <thread>
#include <vector>
#include <iostream>
int main() {
    RWSpinLock rwsl;
    MCSSpinLock mcssl;
    std::vector<std::thread> threads;
    int num = 0;
    for (int i=0;i<10;i++) {
        std::thread t([i, &rwsl, &num, &mcssl]{
            while (true) {
                if (i!=1) {
                    rwsl.rLock();
                    int i = num;
                    mcssl.lock();
                    std::cout << "read: " << num << std::endl;
                    mcssl.unlock();
                    rwsl.rUnLock();
                } else {
                    rwsl.wLock();
                    num++;
                    std::cout << "write: " << num << std::endl;
                    rwsl.wUnLock();
                }

            }
        });
        threads.emplace_back(std::move(t));
    }
    while (true);
}





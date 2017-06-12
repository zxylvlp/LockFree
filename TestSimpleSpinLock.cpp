#include "SimpleSpinLock.h"
#include "vector"
#include "thread"
#include "iostream"
int main(void) {
    SimpleSpinLock ssl;
    std::vector<std::thread> threads;
    int num = 10;
    for (int i=0;i<10;i++) {
        std::thread t([i, &ssl, &num]{
            while (num!=10) {
                PAUSE();
            }
            for (int j=0; j<1000000;j++) {
                ssl.lock();
                std::cout << i << std::endl;
                ssl.unlock();
            }
        });
        threads.emplace_back(std::move(t));
    }
    for (int i=0;i<10;i++) {
        threads[i].join();
    }

}

#include "MCSSpinLock.h"
#include "vector"
#include "thread"
#include "iostream"
int main(void) {
    MCSSpinLock mcsl;
    std::vector<std::thread> threads;
    int num = 0;
    for (int i=0;i<10;i++) {
        std::thread t([i, &mcsl, &num]{
            num++;
            while (num!=10) {
                PAUSE();
            }
            for (int j=0; j<1000000;j++) {
                mcsl.lock();
                std::cout << i << std::endl;
                mcsl.unlock();
            }
        });
        threads.emplace_back(std::move(t));
    }
    for (int i=0;i<10;i++) {
        threads[i].join();
    }

}

#ifndef CONCURRENT_QUEUE_WITH_VERSION_H_
#define CONCURRENT_QUEUE_WITH_VERSION_H_
#include "hazard_version.h"
template<typename T>
class ConcurrentQueueWithVersion {
private:
    struct Item {
        T data_;
        Item *volatile next_;
        Item(): next_(nullptr){}
        Item(T data, Item *next) : data_(data),
                next_(next) {
        }
    };
    Item * volatile head_;
    Item * volatile tail_;
    Item dummy_item_;
    HazardVersion<Item> hv_;
public:
    ConcurrentQueueWithVersion() {
        head_ = &dummy_item_;
        tail_ = &dummy_item_;
    }

    void enqueue(const T element, int thread_id) {
        Item *item = new Item(element, nullptr);
        uint64_t handle = hv_.acquire(thread_id);
        while (true) {
            Item *tail = tail_;
            Item *next = tail->next_;
            if (tail != tail_) {
                continue;
            }
            if (next != nullptr) {
                CAS(&tail_, tail, next);
                continue;
            }
            if (CAS(&tail->next_, nullptr, item)) {
                CAS(&tail_, tail, item);
                break;
            }
        }
        hv_.release(thread_id, handle);
    }

    T dequeue(int thread_id) {
        Item *head = nullptr;
        T data;
        uint64_t handle = hv_.acquire(thread_id);
        while (true) {
            head = head_;
            Item *tail = tail_;
            Item *next = head->next_;
            if (head != head_) {
                continue;
            }
            if (next == nullptr) {
                hv_.release(thread_id, handle);
                return T();
            }
            if (head == tail) {
                CAS(&tail_, tail, next);
                continue;
            }
            data = next->data_;
            if (CAS(&head_, head, next)) {
                break;
            }
        }
        hv_.release(thread_id, handle);
        if (head != &dummy_item_) {
            hv_.retire(thread_id, head);
        }
        return data;
    }


};

#endif /* CONCURRENT_QUEUE_WITH_VERSION_H_ */

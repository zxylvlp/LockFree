#ifndef CONCURRENT_QUEUE_H_
#define CONCURRENT_QUEUE_H_
#include "hazard_pointer.h"
template<typename T>
class ConcurrentQueue {
private:
    struct Item {
        T data_;
        Item *volatile next_;
        Item(): next_(nullptr){}
        Item(T data, Item *next) : data_(data),
                next_(next) {
        }
    };
    struct HazardRecord {
        Item *p_;
        int thread_id_;
        HazardPointer<Item> &hp_;
        HazardRecord(HazardPointer<Item> &hp, Item *p, int thread_id) :
                p_(p), thread_id_(thread_id), hp_(hp) {
            hp_.acquire(p_, thread_id_);
        }
        ~HazardRecord() {
            hp_.release(p_, thread_id_);
        }
    };
    Item * volatile head_;
    Item * volatile tail_;
    Item dummy_item_;
    HazardPointer<Item> hp_;
public:
    ConcurrentQueue() {
        head_ = &dummy_item_;
        tail_ = &dummy_item_;
    }

    void enqueue(const T element, int thread_id) {
        Item *item = new Item(element, nullptr);
        while (true) {
            Item *tail = tail_;
            HazardRecord record_tail(hp_, tail, thread_id);
            if (tail != tail_) {
                continue;
            }
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
    }

    T dequeue(int thread_id) {
        Item *head = nullptr;
        T data;
        while (true) {
            head = head_;
            HazardRecord record_head(hp_, head, thread_id);
            if (head != head_) {
                continue;
            }
            Item *tail = tail_;
            Item *next = head->next_;
            HazardRecord record_next(hp_, next, thread_id);
            if (head != head_) {
                continue;
            }
            if (next == nullptr) {
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
        if (head != &dummy_item_) {
            hp_.retire(head, thread_id);
            hp_.reclaim(thread_id);
        }
        return data;
    }


};

#endif /* CONCURRENT_QUEUE_H_ */

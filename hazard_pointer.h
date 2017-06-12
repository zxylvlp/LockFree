#ifndef HAZARD_POINTER_H_
#define HAZARD_POINTER_H_
#include "atomic.h"
template<typename T>
class HazardPointer {
private:
    struct HazardNode {
        T *p_;
        HazardNode *next_;
        HazardNode() :
                p_(nullptr), next_(nullptr) {
        }
        HazardNode(T *p, HazardNode *next) :
                p_(p), next_(next) {
        }
    };
    struct HazardList {
        uint64_t len_;
        uint64_t cap_;
        HazardNode list_;
        HazardList() :
                len_(0), cap_(0) {
        }
    }CACHE_ALIGNED;
public:
    int acquire(T *p, int thread_id);
    int release(T *p, int thread_id);
    int retire(T *p, int thread_id);
    int reclaim(int thread_id);
    int help(HazardList *list, T *p, int thread_id);
private:
    static const uint64_t MAX_THREAD_NUM = 10;
    static const uint64_t MIN_LEN_TO_RECLAIM = 10;
    HazardList use_list_[MAX_THREAD_NUM];
    HazardList retire_list_[MAX_THREAD_NUM];
};

template<typename T>
int HazardPointer<T>::acquire(T *p, int thread_id) {
    return help(use_list_, p, thread_id);
}

template<typename T>
int HazardPointer<T>::release(T *p, int thread_id) {
    for (HazardNode *node = &use_list_[thread_id].list_; node != nullptr; node =
            node->next_) {
        if (node->p_ == p) {
            node->p_ = nullptr;
            use_list_[thread_id].len_--;
            return 0;
        }
    }
    return -1;
}

template<typename T>
int HazardPointer<T>::retire(T *p, int thread_id) {
    return help(retire_list_, p, thread_id);
}

template<typename T>
int HazardPointer<T>::reclaim(int thread_id) {
    if (retire_list_[thread_id].len_ < MIN_LEN_TO_RECLAIM) {
        return 0;
    }
    for (HazardNode *retire_node = &retire_list_[thread_id].list_;
            retire_node != nullptr; retire_node = retire_node->next_) {
        if (retire_node->p_ == nullptr) {
            continue;
        }
        bool found = false;
        for (int other_thread_id = 0; other_thread_id < MAX_THREAD_NUM;
                other_thread_id++) {
            if (thread_id == other_thread_id) {
                continue;
            }
            for (HazardNode *other_use_node = &use_list_[other_thread_id].list_;
                    other_use_node != nullptr;
                    other_use_node = other_use_node->next_) {
                if (other_use_node->p_ == retire_node->p_) {
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
        if (!found) {
            retire_list_[thread_id].len_--;
            delete retire_node->p_;
            retire_node->p_ = nullptr;
        }
    }
    return 0;
}

template<typename T>
int HazardPointer<T>::help(HazardList *list, T *p, int thread_id) {
    if (p == nullptr) {
        return 0;
    }
    bool found = false;
    HazardNode *empty_node = nullptr;
    for (empty_node = &list[thread_id].list_; empty_node != nullptr;
            empty_node = empty_node->next_) {
        if (empty_node->p_ == nullptr) {
            found = true;
            break;
        }
    }
    if (found) {
        empty_node->p_ = p;
        list[thread_id].len_++;
    } else {
        HazardNode *new_node = new HazardNode(p, list[thread_id].list_.next_);
        MEM_BARRIER();
        list[thread_id].list_.next_ = new_node;
        list[thread_id].len_++;
        list[thread_id].cap_++;
    }
    return 0;
}

#endif /* HAZARD_POINTER_H_ */

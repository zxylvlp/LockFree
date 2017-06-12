#ifndef HAZARD_VERSION_H_
#define HAZARD_VERSION_H_
#include "atomic.h"
#include <assert.h>
#include "SimpleSpinLock.h"
#include <iostream>
#include <sys/time.h>
template<typename T>
class HazardVersion {
    struct Node {
        Node *next_;
        T *data_;
        uint64_t version_;
        Node(T *data);
        ~Node();
    };

    struct ThreadStore {
        uint64_t version_;
        uint64_t seq_;
        Node *retire_list_;
        uint64_t retire_list_len_;
        uint16_t tid_;
        bool enabled_;
        uint64_t last_reclaim_version_;
        ThreadStore *next_;
        ThreadStore();
        ~ThreadStore();
        uint64_t acquire(const uint64_t version, const uint16_t tid);
        void release(const uint64_t tid, uint64_t &handle);
        void add_node(uint64_t version, Node *node);
        uint64_t reclaim(const uint64_t version, ThreadStore *node_receiver);
        void add_nodes(Node *head, Node *tail, const int64_t cnt);
    };
public:
    void reclaim(uint16_t tid);
    uint64_t acquire(uint16_t tid);
    void retire(uint16_t tid, T *p);
    void release(uint16_t tid, uint64_t handle);
    HazardVersion() :
            min_num_to_reclaim_(64), threads_list_(nullptr), threads_cnt_(0), min_version_(
                    0), min_version_ts_(0), min_version_cache_interval_(200000), version_(
                    0), retire_cnt_(0) {

    }
private:
    int64_t tv_to_usec(const timeval &tv);
    int64_t now();
    uint64_t get_min_version(bool force);
    ThreadStore* get_thread_store(int16_t tid) {
        if (tid > MAX_THREAD_CNT) {
            assert(false);
        }
        ThreadStore *ts = &threads_[tid];
        if (!ts->enabled_) {
            threads_lock_.lock();
            if (!ts->enabled_) {
                ts->tid_ = tid;
                ts->version_ = UINT64_MAX;
                ts->next_ = threads_list_;
                ATOMIC_STORE(&threads_list_, ts);
                ADD_AND_FETCH(&threads_cnt_, 1);
                ts->enabled_ = true;
            }
            threads_lock_.unlock();
        }
        return ts;
    }
    const static uint16_t MAX_THREAD_CNT = 10;
    uint64_t min_num_to_reclaim_;
    SimpleSpinLock threads_lock_;
    ThreadStore threads_[MAX_THREAD_CNT];
    ThreadStore *threads_list_;
    uint64_t threads_cnt_;
    uint64_t min_version_;
    int64_t min_version_ts_;
    int64_t min_version_cache_interval_;
    uint64_t version_;
    uint64_t retire_cnt_;
};
template<typename T>
HazardVersion<T>::Node::Node(T *data) :
        next_(nullptr), version_(UINT64_MAX), data_(data) {
}
template<typename T>
HazardVersion<T>::Node::~Node() {
    delete data_;
}
template<typename T>
HazardVersion<T>::ThreadStore::ThreadStore() :
        enabled_(false), tid_(0), last_reclaim_version_(0), seq_(0), version_(
                0), retire_list_(nullptr), retire_list_len_(0), next_(nullptr) {
}
template<typename T>
HazardVersion<T>::ThreadStore::~ThreadStore() {
    while (retire_list_ != nullptr) {
        Node *retire_node = retire_list_;
        retire_list_ = retire_list_->next_;
        delete retire_node;
    }
}
template<typename T>
uint64_t HazardVersion<T>::ThreadStore::acquire(const uint64_t version,
        const uint16_t tid) {
    if (version_ != UINT64_MAX || tid_ != tid) {
        assert(false);
    }
    version_ = version;
    return seq_;
}
template<typename T>
void HazardVersion<T>::ThreadStore::release(const uint64_t tid,
        uint64_t &handle) {
    if (seq_ != handle || tid != tid_) {
        assert(false);
    }
    version_ = UINT64_MAX;
    seq_++;
}
template<typename T>
void HazardVersion<T>::ThreadStore::add_nodes(Node *head, Node *tail,
        const int64_t cnt) {
    if (cnt > 0) {
        Node *curr = ATOMIC_LOAD(&retire_list_);
        Node *old = curr;
        tail->next_ = old;
        while (old != (curr = CAS_RET_VAL(&retire_list_, old, head))) {
            old = curr;
            tail->next_ = old;
        }
        ADD_AND_FETCH(&retire_list_len_, cnt);
    }
}
template<typename T>
void HazardVersion<T>::ThreadStore::add_node(uint64_t version, Node *node) {
    node->version_ = version;
    add_nodes(node, node, 1);
}
template<typename T>
uint64_t HazardVersion<T>::ThreadStore::reclaim(const uint64_t version,
        ThreadStore *node_receiver) {
    if (last_reclaim_version_ == version) {
        return 0;
    }
    last_reclaim_version_ = version;
    Node *curr = ATOMIC_LOAD(&retire_list_);
    Node *old = curr;
    while (old != (curr = CAS_RET_VAL(&retire_list_, old, nullptr))) {
        old = curr;
    }
    Node *list2free = nullptr;
    uint64_t move_cnt = 0;
    uint64_t free_cnt = 0;
    Node dummy(nullptr);
    dummy.next_ = curr;
    Node *iter = &dummy;
    while (iter->next_ != nullptr) {
        if (iter->next_->version_ <= version) {
            free_cnt++;
            Node *node2free = iter->next_;
            iter->next_ = iter->next_->next_;
            node2free->next_ = list2free;
            list2free = node2free;
        } else {
            move_cnt++;
            iter = iter->next_;
        }
    }
    node_receiver->add_nodes(dummy.next_, iter, move_cnt);
    ADD_AND_FETCH(&retire_list_len_, -(move_cnt + free_cnt));
    while (list2free != nullptr) {
        Node *node2free = list2free;
        list2free = list2free->next_;
        delete node2free;
    }
    return free_cnt;
}

template<typename T>
int64_t HazardVersion<T>::tv_to_usec(const timeval &tv) {
        return (((int64_t) tv.tv_sec) * 1000000 + (int64_t) tv.tv_usec);
    }
template<typename T>
int64_t HazardVersion<T>::now() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv_to_usec(tv);
}
template<typename T>
uint64_t HazardVersion<T>::get_min_version(bool force) {
    uint64_t ret = 0;
    int64_t now_ts = now();
    if (!force && (ret = ATOMIC_LOAD(&min_version_)) != 0
            && ATOMIC_LOAD(&min_version_ts_) + min_version_cache_interval_
                    > now_ts) {
        return ret;
    }
    ret = ATOMIC_LOAD(&version_);
    ThreadStore *iter = ATOMIC_LOAD(&threads_list_);
    while (iter != nullptr) {
        uint64_t ts_version = iter->version_;
        if (ret > ts_version) {
            ret = ts_version;
        }
        iter = iter->next_;
    }
    ATOMIC_STORE(&min_version_, ret);
    ATOMIC_STORE(&min_version_ts_, now_ts);
    return ret;
}

template<typename T>
void HazardVersion<T>::reclaim(uint16_t tid) {
    ThreadStore *ts = get_thread_store(tid);
    uint64_t min_version = get_min_version(true);
    int64_t reclaim_cnt = ts->reclaim(min_version, ts);
    ADD_AND_FETCH(&retire_cnt_, -reclaim_cnt);
    ThreadStore *iter = ATOMIC_LOAD(&threads_list_);
    while (iter != nullptr) {
        if (iter != ts) {
            reclaim_cnt = iter->reclaim(min_version, ts);
            ADD_AND_FETCH(&retire_cnt_, -reclaim_cnt);
        }
        iter = iter->next_;
    }
}
template<typename T>
uint64_t HazardVersion<T>::acquire(uint16_t tid) {
    ThreadStore *ts = get_thread_store(tid);
    uint64_t version = ATOMIC_LOAD(&version_);
    uint64_t handle = ts->acquire(version, tid);
    return handle;
}
template<typename T>
void HazardVersion<T>::retire(uint16_t tid, T *p) {
    Node *node = new Node(p);
    ThreadStore *ts = get_thread_store(tid);
    ts->add_node(ADD_AND_FETCH(&version_, 1), node);
    ADD_AND_FETCH(&retire_cnt_, 1);
}

template<typename T>
void HazardVersion<T>::release(uint16_t tid, uint64_t handle) {
    if (tid > MAX_THREAD_CNT) {
        assert(false);
    }
    ThreadStore *ts = get_thread_store(tid);
    ts->release(tid, handle);
    if (ts->retire_list_len_ > min_num_to_reclaim_) {
        uint64_t min_version = get_min_version(false);
        uint64_t reclaim_cnt = ts->reclaim(min_version, ts);
        ADD_AND_FETCH(&retire_cnt_, -reclaim_cnt);
    } else if (ATOMIC_LOAD(&retire_cnt_)
            > ATOMIC_LOAD(&threads_cnt_) * min_num_to_reclaim_) {
        reclaim(tid);
    }
}
#endif /* HAZARD_VERSION_H_ */

#pragma on
// 必须包含的头文件
#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <algorithm>
#include <iostream>

using namespace std;

// 时间类型定义
typedef std::chrono::steady_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

// 定时器节点结构体
struct timernode {
    int socket_id;
    TimeStamp timepoint;      // 过期时间点
    function<void()> cb_func; // 回调函数

    // 运算符重载：用于比较
    bool operator<(const timernode& t) const {
        return timepoint < t.timepoint;
    }
};

class minmap {
private:
    vector<timernode> heap_;            // 堆数组 (物理存储)
    unordered_map<int, int> ref_;       // 映射表: id -> index (逻辑导航)

    // 【核心辅助函数】交换节点，并同时刷新哈希表
    // 只有这里统一处理，才不会乱！
    void swapsi(int i, int j) {
        // 1. 物理交换
        std::swap(heap_[i], heap_[j]);

        // 2. 更新哈希表 (告诉地图，这两家搬家了)
        ref_[heap_[i].socket_id] = i;
        ref_[heap_[j].socket_id] = j;
    }

    // 上浮
    void siftup_(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (heap_[i] < heap_[parent]) {
                swapsi(i, parent);
                i = parent;
            }
            else {
                break;
            }
        }
    }

    // 下沉
    void siftdown_(int index) {
        int i = index;
        int n = heap_.size();
        while (true) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int smallest = i;

            if (left < n && heap_[left] < heap_[smallest])
                smallest = left;
            if (right < n && heap_[right] < heap_[smallest])
                smallest = right;

            if (smallest == i) break;

            swapsi(i, smallest);
            i = smallest;
        }
    }

public:
    // 添加定时器 (带防重复保险)
    void insert(int id, int timeout, function<void()> cb) {
        // 保险：如果这个 ID 已经在堆里了 (比如 Socket 复用)，直接更新它，不要重复插入！
        if (ref_.count(id)) {
            adjust(id, timeout);
            // 更新回调函数 (虽然通常是一样的)
            heap_[ref_[id]].cb_func = cb;
            return;
        }

        // 1. 记录到哈希表
        ref_[id] = heap_.size();

        // 2. 放入堆尾
        TimeStamp t = Clock::now() + MS(timeout);
        heap_.push_back({ id, t, cb });

        // 3. 上浮
        siftup_(heap_.size() - 1);
    }

    // 续命：调整过期时间
    void adjust(int id, int timeout) {
        // 1. 找不到就滚蛋 (防止越界的核心检查)
        if (ref_.find(id) == ref_.end()) return;

        // 2. 拿到下标
        int index = ref_[id];

        // 3. 更新时间
        heap_[index].timepoint = Clock::now() + MS(timeout);

        // 4. 下沉 (因为时间变大了，肯定往后排)
        siftdown_(index);
    }

    // 删除堆顶
    void pop() {
        if (heap_.empty()) return;

        int id_to_remove = heap_[0].socket_id;

        // 1. 交换堆顶和堆尾
        swapsi(0, heap_.size() - 1);

        // 2. 删除映射
        ref_.erase(id_to_remove);

        // 3. 删除物理节点
        heap_.pop_back();

        // 4. 下沉新的堆顶
        if (!heap_.empty()) {
            siftdown_(0);
        }
    }

    // 心跳检测
    void tick() {
        if (heap_.empty()) return;

        while (!heap_.empty()) {
            timernode& node = heap_.front();

            // 还没过期，后面的更不用看了
            if (Clock::now() < node.timepoint) {
                break;
            }

            // 执行回调 (关闭 Socket)
            if (node.cb_func) node.cb_func();

            // 踢掉
            pop();
        }
    }

    // 获取下一次超时时间 (给 select 用)
    int getNextTimeout() {
        if (heap_.empty()) return -1;

        TimeStamp now = Clock::now();
        TimeStamp expire = heap_.front().timepoint;

        auto duration = std::chrono::duration_cast<MS>(expire - now);
        int t = duration.count();

        return (t < 0) ? 0 : t;
    }
};
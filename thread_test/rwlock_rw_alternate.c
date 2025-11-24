#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// 定义全局读写锁
pthread_rwlock_t rwlock;
// 共享资源
int shared_data = 0;

/**
 * @brief 读线程函数
 * 特性：
 * 1. 只要没有线程持有"写锁"，多个读线程可以同时进入临界区（共享锁）。
 * 2. 如果有线程持有"写锁"或正在请求"写锁"，读线程会被阻塞。
 */
void* lock_reader(void* argv) {
  // 申请读锁
  pthread_rwlock_rdlock(&rwlock);

  // --- 临界区 (只读) ---
  // 这里的打印可能会乱序，但读到的 value 必定是当前时刻 shared_data
  // 的一致性快照
  printf("this is %s, value is %d\n", (char*)argv, shared_data);

  // 释放读锁
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

/**
 * @brief 写线程函数
 * 特性：
 * 1. 独占锁：加锁期间，不允许任何其他线程（读或写）进入。
 * 2. 只有当锁处于完全空闲状态时，才能成功加上写锁。
 */
void* lock_writer(void* argv) {
  // 申请写锁
  pthread_rwlock_wrlock(&rwlock);

  // --- 临界区 (修改) ---
  // 在这里进行修改是安全的，不用担心有读者读到修改了一半的数据
  int tmp = shared_data + 1;
  shared_data = tmp;

  printf("this is %s, shared_data++\n", (char*)argv);

  // 释放写锁
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

int main() {
  // 1. 初始化读写锁
  pthread_rwlock_init(&rwlock, NULL);

  pthread_t writer1, writer2, reader1, reader2, reader3, reader4, reader5,
      reader6;

  // 2. 混合创建线程 (高并发模拟)
  // 注意：pthread_create 只是告诉系统"去创建一个线程"，
  // 线程真正的启动时间和顺序由 OS
  // 调度器决定，并不一定严格按照代码行的顺序执行。

  // 启动 Writer1
  pthread_create(&writer1, NULL, lock_writer, "writer1");

  // 紧接着启动 3 个 Reader
  // 如果 Writer1 抢到了锁，这 3 个 Reader 会被阻塞，直到 Writer1 解锁。
  // 如果 Writer1 还没被调度，这 3 个 Reader 可能先抢到锁，导致 Writer1 阻塞。
  pthread_create(&reader1, NULL, lock_reader, "reader1");
  pthread_create(&reader2, NULL, lock_reader, "reader2");
  pthread_create(&reader3, NULL, lock_reader, "reader3");

  // 启动 Writer2
  // 写锁优先级通常较高（避免写饥饿），但具体依赖系统实现。
  pthread_create(&writer2, NULL, lock_writer, "writer2");

  // 启动剩下 3 个 Reader
  pthread_create(&reader4, NULL, lock_reader, "reader4");
  pthread_create(&reader5, NULL, lock_reader, "reader5");
  pthread_create(&reader6, NULL, lock_reader, "reader6");

  // 3. 等待所有线程结束
  pthread_join(writer1, NULL);
  pthread_join(writer2, NULL);
  pthread_join(reader1, NULL);
  pthread_join(reader2, NULL);
  pthread_join(reader3, NULL);
  pthread_join(reader4, NULL);
  pthread_join(reader5, NULL);
  pthread_join(reader6, NULL);

  // 4. 销毁锁
  pthread_rwlock_destroy(&rwlock);

  return 0;
}
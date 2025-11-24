#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// 声明全局读写锁
// 读写锁比互斥锁(Mutex)更具并发性，适用于“读多写少”的场景
pthread_rwlock_t rwlock;

// 共享资源 (临界资源)
int shared_data = 0;

/**
 * @brief 读线程任务函数
 * * 特性：
 * 1. 共享锁：允许多个线程同时持有读锁。
 * 2. 如果此时有线程持有写锁，则读线程会被阻塞，直到写锁释放。
 */
void* lock_reader(void* argv) {
  // 申请读锁
  // 如果当前没有“写锁”，这里会立即成功，哪怕其他线程也持有读锁。
  pthread_rwlock_rdlock(&rwlock);

  // --- 进入临界区 (只读) ---
  // 这里的打印可能会与其他读线程并行发生
  printf("这是 %s (读线程), 读到的值是 %d\n", (char*)argv, shared_data);

  // 释放读锁
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

/**
 * @brief 写线程任务函数
 * * 特性：
 * 1. 独占锁：同一时刻只能有一个线程持有写锁。
 * 2. 排他性：持有写锁时，其他任何线程（无论是读还是写）都无法进入临界区。
 */
void* lock_writer(void* argv) {
  // 申请写锁
  // 如果当前有任何锁（无论是读锁还是写锁）被占用，这里都会阻塞等待
  pthread_rwlock_wrlock(&rwlock);

  // --- 进入临界区 (读写/修改) ---

  // 模拟非原子的修改操作
  int tmp = shared_data + 1;

  // 模拟耗时操作
  // 在这 1秒内，因为持有写锁，整个系统无法读取 shared_data，
  // 所有的 reader 线程和其他 writer 线程都会被卡在锁的入口处。
  sleep(1);

  shared_data = tmp;
  printf("这是 %s (写线程), shared_data++ (修改完毕)\n", (char*)argv);

  // 释放写锁
  // 唤醒等待锁的其他线程（可能是读者，也可能是写者，取决于系统的调度策略）
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

int main() {
  // 1. 初始化读写锁
  // 第二个参数 NULL 表示使用默认属性
  pthread_rwlock_init(&rwlock, NULL);

  pthread_t writer1, writer2, reader1, reader2, reader3, reader4, reader5,
      reader6;

  // 2. 创建两个写线程
  // 它们会竞争写锁。
  // 假设 writer1 先拿到锁，writer2 就会等待，直到 writer1 解锁。
  pthread_create(&writer1, NULL, lock_writer, "writer1");
  pthread_create(&writer2, NULL, lock_writer, "writer2");

  // 3. 主线程休眠 3 秒
  // 这是一个简单的同步策略：给写线程足够的时间（1s + 1s = 2s）去完成修改。
  // 这样能保证下面的读线程启动时，shared_data 已经被修改为 2 了。
  sleep(3);

  // 4. 创建多个读线程
  // 此时 shared_data 应该是 2。
  // 这 6 个线程启动后，因为使用的是
  // rdlock，它们大概率会同时进入临界区并行打印。
  pthread_create(&reader1, NULL, lock_reader, "reader1");
  pthread_create(&reader2, NULL, lock_reader, "reader2");
  pthread_create(&reader3, NULL, lock_reader, "reader3");
  pthread_create(&reader4, NULL, lock_reader, "reader4");
  pthread_create(&reader5, NULL, lock_reader, "reader5");
  pthread_create(&reader6, NULL, lock_reader, "reader6");

  // 5. 等待所有线程结束 (资源回收)
  pthread_join(writer1, NULL);
  pthread_join(writer2, NULL);
  pthread_join(reader1, NULL);
  pthread_join(reader2, NULL);
  pthread_join(reader3, NULL);
  pthread_join(reader4, NULL);
  pthread_join(reader5, NULL);
  pthread_join(reader6, NULL);

  // 6. 销毁读写锁
  pthread_rwlock_destroy(&rwlock);

  return 0;
}
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>  // 用于 sleep 函数

// 定义全局读写锁
pthread_rwlock_t rwlock;
// 共享数据
int shared_data = 0;

/**
 * @brief 读线程函数
 * * 重点观察：
 * 这里加了 sleep(1)，模拟读操作非常耗时。
 * 如果是互斥锁(Mutex)，R1 睡的时候，R2 必须在门外等，导致串行化（耗时累加）。
 * 但因为是【读写锁】，即使 R1 在睡觉且没解锁，R2、R3 依然能拿到读锁进来。
 * 现象：你会发现多个读线程几乎同时打印数据，然后一起睡觉。
 */
void* lock_reader(void* argv) {
  // 1. 申请读锁 (共享锁)
  // 只要当前没有人持有"写锁"，哪怕有一万个线程持有"读锁"，这里也能立马通过
  pthread_rwlock_rdlock(&rwlock);

  // --- 进入临界区 ---
  printf("this is %s, value is %d\n", (char*)argv, shared_data);

  // 模拟耗时的读取操作 (关键点)
  // 在这 1 秒内，当前线程持有读锁。
  // 此时，Writer 线程会被完全阻塞（无法拿写锁）。
  // 但是！其他的 Reader 线程可以畅通无阻地进入这里。
  sleep(1);

  // 2. 释放读锁
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

/**
 * @brief 写线程函数
 * * 重点观察：
 * 写锁是独占的。一旦拿到写锁，没有任何人（读或写）能进来。
 * 如果上面的 Reader 正在 sleep，Writer 必须在门外死等，直到所有 Reader
 * 都释放锁。
 */
void* lock_writer(void* argv) {
  // 1. 申请写锁 (独占锁)
  // 如果此时有任何 Reader 在 sleep，这里会阻塞
  pthread_rwlock_wrlock(&rwlock);

  // --- 进入临界区 ---
  // 修改数据
  int tmp = shared_data + 1;
  shared_data = tmp;

  printf("this is %s, shared_data++\n", (char*)argv);

  // 2. 释放写锁
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

int main() {
  // 初始化读写锁
  pthread_rwlock_init(&rwlock, NULL);

  pthread_t writer1, writer2, reader1, reader2, reader3, reader4, reader5,
      reader6;

  // ---------------- 创建线程 (模拟混合并发) ----------------

  // 1. 启动 Writer1
  // 很大几率它先抢到锁，把 shared_data 改为 1
  pthread_create(&writer1, NULL, lock_writer, "writer1");

  // 2. 启动一批 Reader (R1, R2, R3)
  // 假设 Writer1 释放后，这三个 Reader 抢到了锁。
  // 它们会【并发】执行：
  // R1 进来了 -> 打印 -> sleep
  // R2 进来了 -> 打印 -> sleep (不必等 R1 醒来)
  // R3 进来了 -> 打印 -> sleep (不必等 R2 醒来)
  // 结果：你会看到三条打印瞬间出来，然后程序停顿 1 秒。
  pthread_create(&reader1, NULL, lock_reader, "reader1");
  pthread_create(&reader2, NULL, lock_reader, "reader2");
  pthread_create(&reader3, NULL, lock_reader, "reader3");

  // 3. 启动 Writer2
  // 这是一个倒霉的写线程。
  // 当它尝试加写锁时，如果 R1, R2, R3 还在那睡着（持有读锁），
  // Writer2 就必须阻塞等待，直到 R1, R2, R3 全部醒来并 unlock。
  pthread_create(&writer2, NULL, lock_writer, "writer2");

  // 4. 启动下一批 Reader
  // 只有等 Writer2 写完释放后，这些 Reader 才能读到最新的值 (2)
  pthread_create(&reader4, NULL, lock_reader, "reader4");
  pthread_create(&reader5, NULL, lock_reader, "reader5");
  pthread_create(&reader6, NULL, lock_reader, "reader6");

  // ---------------- 等待回收 ----------------
  pthread_join(writer1, NULL);
  pthread_join(writer2, NULL);
  pthread_join(reader1, NULL);
  pthread_join(reader2, NULL);
  pthread_join(reader3, NULL);
  pthread_join(reader4, NULL);
  pthread_join(reader5, NULL);
  pthread_join(reader6, NULL);

  // 销毁锁
  pthread_rwlock_destroy(&rwlock);

  return 0;
}
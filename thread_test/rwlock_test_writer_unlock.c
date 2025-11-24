#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// 声明全局的读写锁
pthread_rwlock_t rwlock;
// 共享资源
int shared_data = 0;

/**
 * @brief 读线程函数
 * 演示获取读锁
 */
void* lock_reader(void* argv) {
  // 1. 加读锁 (Read Lock)
  // 特性：如果有其他线程持有读锁，这里不会阻塞，直接进入（共享）。
  //      如果有线程持有写锁，这里会阻塞等待。
  pthread_rwlock_rdlock(&rwlock);

  // --- 临界区 (只读操作) ---
  printf("this is %s, value is %d\n", (char*)argv, shared_data);

  // 2. 解锁
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

/**
 * @brief 写线程函数
 * ⚠️⚠️⚠️ 注意：你提供的原代码中，这里缺少了加锁逻辑！
 * 正常情况下，写操作必须加“写锁”才能保证数据安全。
 */
void* lock_writer(void* argv) {
  // [缺失] 这里应该调用 pthread_rwlock_wrlock(&rwlock);

  // --- 临界区 (修改操作) ---
  // 模拟非原子的修改过程：先读取，休眠，再赋值
  // 如果没有写锁，两个写线程并发执行到这里会发生“竞态条件”，导致数据只增加 1
  // 而不是 2
  int tmp = shared_data + 1;

  sleep(1);  // 模拟耗时的写操作

  shared_data = tmp;
  printf("this is %s, shared_data++\n", (char*)argv);

  // [缺失] 这里应该调用 pthread_rwlock_unlock(&rwlock);
  return NULL;
}

int main() {
  // 1. 初始化读写锁
  // 第二个参数 NULL 表示使用默认属性
  pthread_rwlock_init(&rwlock, NULL);

  pthread_t writer1, writer2, reader1, reader2, reader3, reader4, reader5,
      reader6;

  // 2. 创建两个写线程
  // 注意：由于 lock_writer 里面没加锁，这两个线程会同时修改
  // shared_data，引发竞争
  pthread_create(&writer1, NULL, lock_writer, "writer1");
  pthread_create(&writer2, NULL, lock_writer, "writer2");

  // 3. 主线程休眠 3 秒
  // 目的：让两个写线程先跑完。如果不休眠，读线程可能会在写线程还没写完时读到旧数据（或脏数据）。
  sleep(3);

  // 4. 创建多个读线程
  // 这些读线程由于使用的是 rdlock，它们可以并行打印数据，不需要互相等待
  pthread_create(&reader1, NULL, lock_reader, "reader1");
  pthread_create(&reader2, NULL, lock_reader, "reader2");
  pthread_create(&reader3, NULL, lock_reader, "reader3");
  pthread_create(&reader4, NULL, lock_reader, "reader4");
  pthread_create(&reader5, NULL, lock_reader, "reader5");
  pthread_create(&reader6, NULL, lock_reader, "reader6");

  // 5. 等待回收所有线程
  pthread_join(writer1, NULL);
  pthread_join(writer2, NULL);
  pthread_join(reader1, NULL);
  pthread_join(reader2, NULL);
  pthread_join(reader3, NULL);
  pthread_join(reader4, NULL);
  pthread_join(reader5, NULL);
  pthread_join(reader6, NULL);

  // 6. 销毁读写锁，释放资源
  pthread_rwlock_destroy(&rwlock);

  return 0;
}
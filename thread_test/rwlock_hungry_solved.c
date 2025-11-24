#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// 定义全局读写锁
pthread_rwlock_t rwlock;
// 共享资源
int shared_data = 0;

/**
 * @brief 读线程函数
 */
void* lock_reader(void* argv) {
  // 申请读锁
  pthread_rwlock_rdlock(&rwlock);

  printf("this is %s, value is %d\n", (char*)argv, shared_data);

  // 模拟耗时操作
  // 即使在这里睡觉，如果策略是"写者优先"且有写者在等待，
  // 那么新的读者将被阻塞在 rwlock_rdlock 处，不能进来插队。
  sleep(1);

  // 释放读锁
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

/**
 * @brief 写线程函数
 */
void* lock_writer(void* argv) {
  // 申请写锁
  pthread_rwlock_wrlock(&rwlock);

  // 修改数据
  int tmp = shared_data + 1;
  shared_data = tmp;
  printf("this is %s, shared_data++\n", (char*)argv);

  // 释放写锁
  pthread_rwlock_unlock(&rwlock);
  return NULL;
}

int main() {
  // 1. 定义并初始化读写锁属性对象
  pthread_rwlockattr_t attr;
  pthread_rwlockattr_init(&attr);

  // 2. 设置读写锁的“偏好”为写者优先 (关键步骤)
  // 函数名中的 _np 后缀表示 "Non-Portable" (不可移植)，这是 Linux Glibc
  // 特有的扩展。
  //
  // 参数 PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP 的含义：
  // - PREFER_WRITER:
  // 写者优先。如果有一个写者在等待锁，那么后续到达的读者将无法获取读锁，
  //                  必须等待写者完成。这避免了写者被源源不断的读者饿死。
  // - NONRECURSIVE:  写锁不允许递归加锁 (同一个线程不能多次加写锁)。
  pthread_rwlockattr_setkind_np(&attr,
                                PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);

  // 3. 使用设置好的属性初始化读写锁
  // 注意：第二个参数不再是 NULL，而是我们要的 attr
  pthread_rwlock_init(&rwlock, &attr);

  // 属性对象用完后销毁
  pthread_rwlockattr_destroy(&attr);

  pthread_t writer1, writer2, reader1, reader2, reader3, reader4, reader5,
      reader6;

  // 4. 创建线程演示

  // 启动 writer1 (它应该能最先抢到)
  pthread_create(&writer1, NULL, lock_writer, "writer1");

  // 启动一批 reader
  // 假设 writer1 还在跑或者刚跑完，这几个 reader 进去了，并且每个人都 sleep(1)
  pthread_create(&reader1, NULL, lock_reader, "reader1");
  pthread_create(&reader2, NULL, lock_reader, "reader2");
  pthread_create(&reader3, NULL, lock_reader, "reader3");

  // 启动 writer2 (关键观察点)
  // 此时 reader1/2/3 可能还持有锁（因为在 sleep）。
  // 在默认模式下，如果此时 reader4 来了，它能直接进去读。
  // 但在【写者优先】模式下，因为 writer2 在排队，系统会把门关上，不让
  // reader4/5/6 进。
  pthread_create(&writer2, NULL, lock_writer, "writer2");

  // 启动后续 reader
  // 这些 reader 理论上会被 writer2 挡在后面，直到 writer2 写完才能读
  pthread_create(&reader4, NULL, lock_reader, "reader4");
  pthread_create(&reader5, NULL, lock_reader, "reader5");
  pthread_create(&reader6, NULL, lock_reader, "reader6");

  // 5. 等待回收
  pthread_join(writer1, NULL);
  pthread_join(writer2, NULL);
  pthread_join(reader1, NULL);
  pthread_join(reader2, NULL);
  pthread_join(reader3, NULL);
  pthread_join(reader4, NULL);
  pthread_join(reader5, NULL);
  pthread_join(reader6, NULL);

  // 6. 销毁锁
  pthread_rwlock_destroy(&rwlock);

  return 0;
}
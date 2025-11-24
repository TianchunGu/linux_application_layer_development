#include <pthread.h>  // 线程核心库: 提供 pthread_mutex_t, lock, unlock 等
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 定义并发线程数量
#define THREAD_COUNT 20000

// ---------------- 1. 定义并初始化互斥锁 ----------------
// PTHREAD_MUTEX_INITIALIZER 是静态初始化宏。
// 它在编译期就完成了锁的初始化，无需在 main 函数中调用 pthread_mutex_init。
// 这把锁就像是公共卫生间的门锁，同一时间只有一个人能拿到钥匙进去。
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief 线程任务函数：对传入值累加1
 *
 * @param argv 传入的参数指针 (指向 main 函数中的 num)
 * @return void* 无返回值
 */
void* add_thread(void* argv) {
  int* p = argv;

  // ---------------- 2. 加锁 (进入临界区) ----------------
  // pthread_mutex_lock: 尝试获取锁。
  // - 如果锁是开着的：当前线程获得锁，锁状态变为“占用”，继续往下执行。
  // - 如果锁已经被别人锁上了：当前线程会“阻塞” (Block/Sleep)，
  //   暂停执行并在门口排队，直到锁被解开。
  pthread_mutex_lock(&counter_mutex);

  // ---------------- 3. 临界区 (Critical Section) ----------------
  // 这里的代码被保护起来了，具有了“原子性”。
  // 无论有多少个线程在运行，同一时刻，这行代码只能被一个线程执行。
  // 这保证了读-改-写三个步骤不会被其他线程打断。
  (*p)++;

  // ---------------- 4. 解锁 (退出临界区) ----------------
  // pthread_mutex_unlock: 释放锁。
  // 锁状态变为“空闲”。
  // 此时，如果有其他线程卡在上面的 lock 处排队，系统会唤醒其中一个让它进来。
  pthread_mutex_unlock(&counter_mutex);

  return (void*)0;
}

int main() {
  pthread_t pid[THREAD_COUNT];

  // 共享变量
  int num = 0;

  // 1. 创建 20000 个线程
  // 每个线程都试图去抢锁，然后给 num 加 1
  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_create(pid + i, NULL, add_thread, &num);
  }

  // 2. 等待回收所有线程
  // 确保所有人都干完活了，再打印结果
  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(pid[i], NULL);
  }

  // 3. 打印结果
  // 由于加了锁，这一次的结果必然是 20000。
  // 数据竞争被消除了。
  printf("累加结果：%d\n", num);

  // 注意：对于静态初始化的互斥锁 (PTHREAD_MUTEX_INITIALIZER)，
  // 程序结束时通常不需要显式调用 pthread_mutex_destroy。
  // 但如果是用 pthread_mutex_init 动态创建的锁，则必须销毁。

  return 0;
}
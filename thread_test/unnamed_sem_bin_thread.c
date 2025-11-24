#include <pthread.h>
#include <semaphore.h>  // 信号量核心头文件
#include <stdio.h>
#include <unistd.h>

// 定义一个全局的信号量变量
sem_t unnamed_sem;

// 共享资源
int shard_num = 0;

/**
 * @brief 线程执行函数
 * * 此时的操作是线程安全的，因为加了信号量保护。
 */
void* plusOne(void* argv) {
  // ---------------- P操作 (Wait / Lock) ----------------
  // sem_wait 会尝试将信号量的值减 1。
  // 1. 如果当前信号量的值 > 0 (也就是 1)：减为
  // 0，函数立即返回，线程继续执行（拿到锁）。
  // 2. 如果当前信号量的值 == 0：线程被阻塞
  // (Block)，进入睡眠状态，直到信号量变为 > 0。 这相当于“关门”操作。
  sem_wait(&unnamed_sem);

  // ---------------- 临界区 (Critical Section) ----------------
  // 被保护的代码区域。由于信号量初始值为1，同一时刻只有一个线程能进到这里。
  // 哪怕这里发生上下文切换，其他线程也会卡在上面的 sem_wait 处。
  int tmp = shard_num + 1;
  shard_num = tmp;

  // ---------------- V操作 (Post / Signal / Unlock) ----------------
  // sem_post 会将信号量的值加 1。
  // 1. 信号量变回 1。
  // 2. 如果有其他线程正在 sem_wait 处排队等待，系统会唤醒其中一个线程。
  // 这相当于“开门”操作。
  sem_post(&unnamed_sem);

  return NULL;
}

int main() {
  // ---------------- 1. 初始化信号量 ----------------
  // int sem_init(sem_t *sem, int pshared, unsigned int value);
  // 参数 1 (&unnamed_sem): 信号量指针
  // 参数 2 (0): 0 表示信号量在当前进程的线程间共享；非0表示在进程间共享。
  // 参数 3 (1): **关键点**！初始值设为 1。
  //             这被称为“二值信号量” (Binary Semaphore)，作用等同于互斥锁。
  //             表示同一时间只允许 1 个线程进入临界区。
  sem_init(&unnamed_sem, 0, 1);

  pthread_t tid[10000];

  // 2. 创建 10000 个线程
  // 这些线程启动后，会疯狂竞争那个唯一的“通行证” (信号量)
  for (int i = 0; i < 10000; i++) {
    pthread_create(tid + i, NULL, plusOne, NULL);
  }

  // 3. 等待所有线程结束
  for (int i = 0; i < 10000; i++) {
    pthread_join(tid[i], NULL);
  }

  // 4. 打印结果
  // 由于加了锁，所有的累加操作都是原子的。
  // 结果必然是 10000。
  printf("shard_num is %d\n", shard_num);

  // 5. 销毁信号量
  // 释放相关资源
  sem_destroy(&unnamed_sem);

  return 0;
}
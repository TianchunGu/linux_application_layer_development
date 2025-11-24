#include <pthread.h>    // 线程核心头文件
#include <semaphore.h>  // 信号量核心头文件
#include <stdio.h>
#include <stdlib.h>  // malloc, rand, srand
#include <time.h>    // time
#include <unistd.h>  // sleep

// 定义两个信号量指针
// sem_t *full:  代表“缓冲区有多少个数据可读”。初始值为 0。
// sem_t *empty: 代表“缓冲区有多少个空位可写”。初始值为 1。
sem_t* full;
sem_t* empty;

// 共享资源 (缓冲区)
// 这里简化为一个整数，实际上可以是一个数组或队列
int shard_num;

// 生成随机数的辅助函数
// ⚠️ 注意：这里有一个严重的逻辑缺陷（详见代码末尾的解析）
int rand_num() {
  // 设置随机数种子
  srand(time(NULL));
  return rand();
}

/**
 * @brief 生产者线程函数
 * 负责产生数据并放入缓冲区
 */
void* producer(void* argv) {
  for (int i = 0; i < 5; i++) {
    // ---------------- P操作 (Wait) ----------------
    // 等待空位。
    // 初始 empty = 1，第一次执行时会成功，将 empty 减为 0。
    // 如果 empty 已经是 0 (说明上一个数据还没被消费)，这里会阻塞等待。
    sem_wait(empty);

    // --- 进入临界区 (生产数据) ---
    printf("\n==========> 第 %d 轮数据传输 <=========\n\n", i + 1);
    sleep(1);  // 模拟生产耗时

    // 生成数据并写入共享变量
    shard_num = rand_num();
    printf("producer(生产者) has sent data: %d\n", shard_num);

    // ---------------- V操作 (Signal) ----------------
    // 发送信号告知消费者：有数据了！
    // full 的值 +1 (从 0 变为 1)，唤醒阻塞在 full 上的消费者。
    sem_post(full);
  }
  return NULL;
}

/**
 * @brief 消费者线程函数
 * 负责从缓冲区读取数据
 */
void* consumer(void* argv) {
  for (int i = 0; i < 5; i++) {
    // ---------------- P操作 (Wait) ----------------
    // 等待数据。
    // 初始 full = 0，消费者一上来就会在这里阻塞，直到生产者 post(full)。
    sem_wait(full);

    // --- 进入临界区 (消费数据) ---
    printf("consumer(消费者) has read data\n");
    printf("the shard_num is %d\n", shard_num);

    sleep(1);  // 模拟处理数据的耗时

    // ---------------- V操作 (Signal) ----------------
    // 发送信号告知生产者：我读完了，腾出空位了！
    // empty 的值 +1 (从 0 变为 1)，唤醒阻塞在 empty 上的生产者。
    sem_post(empty);
  }
  return NULL;
}

int main() {
  // 1. 动态分配信号量内存
  // 在多线程中，全局指针指向堆内存是可行的。
  // 也可以直接声明全局变量 sem_t full, empty; 不需要 malloc。
  full = malloc(sizeof(sem_t));
  empty = malloc(sizeof(sem_t));

  // 2. 初始化信号量
  // 参数2 (0): 0 表示线程间共享 (Thread-Shared)
  // 参数3 (value):
  //    empty 设为 1: 表示一开始有 1 个空位，允许生产者先运行。
  //    full  设为 0: 表示一开始没有数据，禁止消费者先运行。
  sem_init(empty, 0, 1);
  sem_init(full, 0, 0);

  pthread_t producer_id, consumer_id;

  // 3. 创建线程
  pthread_create(&producer_id, NULL, producer, NULL);
  pthread_create(&consumer_id, NULL, consumer, NULL);

  // 4. 等待线程结束
  pthread_join(producer_id, NULL);
  pthread_join(consumer_id, NULL);

  // 5. 销毁信号量
  sem_destroy(empty);
  sem_destroy(full);

  // 6. 释放 malloc 的内存 (代码原作者遗漏了这一步，最好补上)
  free(empty);
  free(full);

  return 0;
}
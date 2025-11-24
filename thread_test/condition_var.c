#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// 定义缓冲区大小
#define BUFFER_SIZE 5

// ---------------- 全局共享资源 ----------------
// 缓冲区，模拟仓库
int buffer[BUFFER_SIZE];
// 当前缓冲区中的产品数量 (临界资源，需要被保护)
int count = 0;

// ---------------- 同步原语初始化 ----------------
// 初始化互斥锁: 用于保护 count 和 buffer 的原子性操作
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// 初始化条件变量: 用于线程的挂起(wait)和唤醒(signal)
// 注意：本例中只用了一个条件变量，既用于"非满"也用于"非空"
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/**
 * @brief 生产者线程函数
 * 负责向缓冲区写入数据
 */
void* producer(void* arg) {
  int item = 1;
  while (1) {
    // 1. 加锁：进入临界区
    // 在操作全局变量 count 和 buffer 之前必须拿到锁
    pthread_mutex_lock(&mutex);

    // 2. 检查条件：缓冲区是否已满？
    // ⚠️ 关键点：必须使用 while 循环而不是 if
    // 原因防止"虚假唤醒"(Spurious Wakeup)：
    // 线程可能被信号唤醒，但唤醒后发现缓冲区依然是满的（可能被其他生产者抢先了），
    // 所以醒来后必须再次检查条件。
    while (count == BUFFER_SIZE) {
      // 3. 等待：缓冲区满，无法生产
      // pthread_cond_wait 做三件事 (原子操作)：
      //   a. 解锁 mutex (让消费者有机会拿到锁去消费)
      //   b. 阻塞当前线程，进入休眠状态等待 cond 信号
      //   c. 当被唤醒后，重新竞争并加锁 mutex，然后函数返回
      pthread_cond_wait(&cond, &mutex);
    }

    // 4. 生产数据
    // 程序能执行到这里，说明 count < BUFFER_SIZE 且持有锁
    buffer[count++] = item++;
    printf("生产者(白月光)发送一个幸运数字: %d (当前库存: %d)\n",
           buffer[count - 1], count);

    // 5. 唤醒等待的线程
    // 发送信号给在 cond 上等待的线程（通常是消费者，但也可能是其他生产者）
    // 注意：signal 只是通知，不会自动释放锁
    pthread_cond_signal(&cond);

    // 6. 解锁：离开临界区
    pthread_mutex_unlock(&mutex);
  }
}

/**
 * @brief 消费者线程函数
 * 负责从缓冲区读取数据
 */
void* consumer(void* arg) {
  while (1) {
    // 1. 加锁
    pthread_mutex_lock(&mutex);

    // 2. 检查条件：缓冲区是否为空？
    // 同样必须使用 while 防止虚假唤醒
    while (count == 0) {
      // 3. 等待：缓冲区空，无法消费
      // 解锁 -> 等待信号 -> 被唤醒后重新加锁
      pthread_cond_wait(&cond, &mutex);
    }

    // 4. 消费数据
    // 程序执行到这里，说明 count > 0 且持有锁
    printf("消费者收到了幸运数字: %d (当前库存: %d)\n", buffer[--count], count);

    // 5. 唤醒等待的线程（通知生产者可以生产了）
    pthread_cond_signal(&cond);

    // 6. 解锁
    pthread_mutex_unlock(&mutex);
  }
}

int main() {
  pthread_t producer_thread, consumer_thread;

  // 创建生产者和消费者线程
  // 它们并发运行，通过 mutex 和 cond 协调速度
  pthread_create(&producer_thread, NULL, producer, NULL);
  pthread_create(&consumer_thread, NULL, consumer, NULL);

  // 等待线程结束
  // 由于线程内部是 while(1) 死循环，主程序会一直阻塞在这里
  pthread_join(producer_thread, NULL);
  pthread_join(consumer_thread, NULL);

  return 0;
}
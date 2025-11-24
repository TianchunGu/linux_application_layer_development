#include <pthread.h>
#include <semaphore.h>  // 虽然引入了信号量头文件，但本代码中并未实际使用它来保护共享资源
#include <stdio.h>
#include <unistd.h>

// 全局共享变量
// 所有线程都会同时访问和修改这个变量
int shard_num = 0;

/**
 * @brief 线程执行函数：试图给全局变量加 1
 * * ⚠️ 严重警告：这是一个非线程安全的函数！
 * 这里模拟了非原子的“读-改-写”操作，是产生竞态条件的根源。
 */
void* plusOne(void* argv) {
  // ---------------- 临界区 (Critical Section) 开始 ----------------

  // 步骤 1: 读取 (Read)
  // 线程从内存读取 shard_num 的当前值到局部变量 tmp
  int tmp = shard_num + 1;

  // 假设线程 A 读到 0，准备写回 1。
  // 此时发生了上下文切换，线程 B 进来，也读到 0 (因为 A 还没写回)。
  // 线程 B 也准备写回 1。

  // 步骤 2: 写入 (Write)
  // 将局部变量的值写回全局变量
  shard_num = tmp;

  // 结果：线程 A 和 B 都把 shard_num 设为了 1。
  // 两次操作只产生了一次累加效果，这就叫“丢失更新”。

  // ---------------- 临界区 结束 ----------------
  return NULL;
}

int main() {
  // 定义存放 10000 个线程 ID 的数组
  // 注意：栈空间有限，过大的数组可能导致栈溢出，但在现代 Linux 上 10000
  // 通常没问题
  pthread_t tid[10000];

  // 1. 循环创建 10000 个线程
  for (int i = 0; i < 10000; i++) {
    // 创建线程，每个线程都去执行 plusOne 函数
    // 此时，成千上万个线程开始并发竞争 CPU 资源
    pthread_create(tid + i, NULL, plusOne, NULL);
  }

  // 2. 循环等待所有线程结束
  // 必须等待所有线程执行完毕，否则主线程可能先打印结果甚至退出了
  for (int i = 0; i < 10000; i++) {
    pthread_join(tid[i], NULL);
  }

  // 3. 打印最终结果
  // 预期值：10000
  // 实际值：很可能小于 10000 (例如 9854)，且每次运行结果都不一样
  printf("shard_num is %d\n", shard_num);

  return 0;
}
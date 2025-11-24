#include <pthread.h>  // 线程核心头文件
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 定义并发线程的数量
#define THREAD_COUNT 20000

/**
 * @brief 线程任务函数：对传入的整数指针进行累加
 *
 * @param argv 传入参数 (实际上是 main 函数中 num 变量的地址)
 * @return void* 无返回值
 */
void* add_thread(void* argv) {
  // 将 void* 指针还原为 int* 指针
  int* p = argv;

  // ---------------- ⚠️ 关键问题区域 (临界区) ⚠️ ----------------
  // (*p)++ 看似只有一行代码，但在计算机底层（汇编层面）对应三条指令：
  // 1. LOAD:  将内存中 *p 的值读入 CPU 寄存器
  // 2. INC:   将寄存器中的值加 1
  // 3. STORE: 将寄存器的新值写回内存
  //
  // 由于没有加锁（Mutex），多个线程可能同时执行到第 1 步。
  // 例如：线程A读到 num=5，线程B也读到 num=5。
  // 线程A加1变成6写回，线程B加1变成6写回。
  // 结果：两次加法操作，num 只增加了 1。这就是“竞态条件”。
  (*p)++;

  return (void*)0;
}

int main() {
  // 定义数组存放 20000 个线程的 ID
  pthread_t pid[THREAD_COUNT];

  // 定义共享资源变量
  // 注意：这个变量位于 main 函数的栈上，但通过指针传递给了所有子线程共享
  int num = 0;

  // 1. 创建线程
  for (int i = 0; i < THREAD_COUNT; i++) {
    // 参数4 (&num): 将 num 的地址传给子线程
    // 这样所有子线程操作的都是同一个内存地址
    pthread_create(pid + i, NULL, add_thread, &num);
  }

  // 2. 等待回收
  // 必须等待所有线程执行完毕，否则 main 函数直接打印结果时，
  // 还有很多线程可能没来得及执行 (*p)++
  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(pid[i], NULL);
  }

  // 3. 打印结果
  // 预期结果: 20000
  // 实际结果: 通常小于 20000 (例如 19853)，且每次运行结果都不一样
  printf("累加结果：%d\n", num);

  return 0;
}
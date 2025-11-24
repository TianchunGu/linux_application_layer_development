#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * 线程执行函数
 */
void* task(void* arg) {
  printf("[子线程] Thread started (开始运行)\n");

  // 默认情况下，线程的取消类型是 PTHREAD_CANCEL_DEFERRED (延迟取消)
  // 这意味着线程收到取消请求后，不会立即死掉，而是继续运行，
  // 直到遇到下一个“取消点” (Cancellation Point) 时才会退出。
  // 绝大多数系统调用 (如 sleep, read, write, printf 等) 都是取消点。

  printf("[子线程] Working... (正在工作)\n");

  // sleep 也是一个标准的取消点。
  // 如果主线程的 cancel 请求发得够快，线程可能会在 sleep 这里就退出了。
  sleep(1);

  // 手动设置一个取消点
  // pthread_testcancel()
  // 的作用是：“检查当前有没有收到取消请求？如果有，立刻在这里结束线程。”
  // 这常用于计算密集型任务（没有系统调用），为了防止线程无法响应取消，
  // 程序员会在循环中手动插入这个函数。
  pthread_testcancel();

  // 如果取消成功，代码永远不会执行到这里
  printf("[子线程] After Cancelled (如果不应该打印这句话，说明取消失败了)\n");

  return NULL;
}

int main() {
  pthread_t tid;
  void* res;  // 用于接收线程的返回值 (或退出状态)

  // 1. 创建线程
  pthread_create(&tid, NULL, task, NULL);

  // 2. 发送取消请求
  // 注意：pthread_cancel 只是“发送一个请求”，并不是强制杀死线程 (不像 kill
  // -9)。 线程是否退出、何时退出，取决于线程的“取消状态”和“取消类型”。
  if (pthread_cancel(tid) != 0) {
    perror("pthread_cancel");
  }
  printf("[主线程] 已发送取消请求...\n");

  // 3. 等待线程结束并回收资源
  // pthread_join 是必须的，否则会产生僵尸线程。
  // 第二个参数 &res 会存储线程的退出码。
  pthread_join(tid, &res);

  // 4. 检查线程是怎么死的
  // PTHREAD_CANCELED 是一个特殊的宏 (通常定义为 (void*)-1)
  // 只有当线程是因为响应了 pthread_cancel 而退出时，res 才会等于这个值。
  if (res == PTHREAD_CANCELED) {
    printf("[主线程] 确认: 子线程已被取消 (Thread was canceled)\n");
  } else {
    // 如果线程是正常跑完 return 的，或者 pthread_exit 的，会走这里
    printf("[主线程] 意外: 子线程未被取消, 正常退出 code: %ld\n", (long)res);
  }

  return 0;
}
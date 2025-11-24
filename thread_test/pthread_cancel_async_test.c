#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * 线程任务函数
 */
void* task(void* arg) {
  printf("[子线程] Thread started\n");

  // ---------------- 关键设置 ----------------
  // 修改取消类型为：PTHREAD_CANCEL_ASYNCHRONOUS (异步取消)
  // 默认是 PTHREAD_CANCEL_DEFERRED (延迟取消)
  //
  // 含义：一旦收到取消信号，线程可能会在下一条汇编指令执行时立即挂掉，
  // 它不再需要等待 sleep() 或 pthread_testcancel() 等取消点。
  int old_type;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_type);

  // 模拟工作
  printf("[子线程] Working...\n");

  // 在异步模式下，线程可能在打印完上面的字后，
  // 还没来得及执行 sleep(1) 就已经被杀死了。
  // 甚至可能在 printf 函数内部执行到一半时被杀死（这很危险）。
  sleep(1);

  // 如果取消成功，这行代码绝不会被执行
  printf("[子线程] After Cancelled (这就话不应该出现)\n");

  return NULL;
}

int main() {
  pthread_t tid;
  void* res;  // 用于存放线程退出状态

  // 1. 创建线程
  pthread_create(&tid, NULL, task, NULL);

  // 为了演示效果，稍微让主线程停顿一下，保证子线程有机会运行到 setcanceltype
  // 但在这个简单的例子中，即使不加 sleep，只要 cancel
  // 发出时子线程已经跑起来了，就会生效 sleep(1);

  // 2. 发送取消请求
  // 只要子线程运行了 pthread_setcanceltype，它就会立刻响应该请求并终止
  if (pthread_cancel(tid) != 0) {
    perror("pthread_cancel");
  }
  printf("[主线程] Sent cancel request (已发送取消请求)\n");

  // 3. 等待子线程终止 (收尸)
  // 这一步是必须的，否则子线程资源无法回收
  pthread_join(tid, &res);

  // 4. 检查退出状态
  // PTHREAD_CANCELED 是个宏，通常值为 (void*)-1
  if (res == PTHREAD_CANCELED) {
    printf("[主线程] Success: Thread was canceled (子线程已被成功取消)\n");
  } else {
    printf("[主线程] Failed: Thread was not canceled, exit code: %ld\n",
           (long)res);
  }

  return 0;
}
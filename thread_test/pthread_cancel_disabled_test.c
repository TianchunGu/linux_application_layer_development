#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * 线程任务函数
 */
void* task(void* arg) {
  printf("[子线程] Thread started\n");

  // ---------------- 1. 禁用取消 (关键步骤) ----------------
  // PTHREAD_CANCEL_DISABLE: 告诉系统“我不听，我不听”。
  // 此时，任何针对该线程的 pthread_cancel 请求都会被“挂起” (Pending)，
  // 而不会立即生效。线程会继续执行，直到它主动将状态改回 ENABLE。
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  printf("[子线程] Thread CancelState is disabled (已开启防取消护盾)\n");

  // ---------------- 2. 设置异步取消类型 ----------------
  // PTHREAD_CANCEL_ASYNCHRONOUS: 本意是收到信号立即死。
  // 但是！由于上面已经设置了 DISABLE，这个设置虽然生效了，但在 DISABLE
  // 解除之前， 它是没有任何杀伤力的。 如果后续代码调用了
  // setcancelstate(ENABLE)，线程会在那一瞬间立即死亡。
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  // 模拟工作
  printf("[子线程] Working...\n");

  // 即使 sleep 是一个标准的取消点，且类型是异步，
  // 但因为状态是 DISABLE，线程不会在这里停止，而是继续睡完 1 秒。
  sleep(1);

  // 这行代码会被执行，证明线程没有被杀死
  printf("[子线程] After Cancelled (如果你看到这句，说明护盾生效了)\n");

  return NULL;  // 正常返回 NULL
}

int main() {
  pthread_t tid;
  void* res;

  // 创建线程
  pthread_create(&tid, NULL, task, NULL);

  // ---------------- 3. 发送取消请求 ----------------
  // pthread_cancel 的返回值 0 表示“请求发送成功”，
  // 并不代表线程“已经被杀死了”。
  // 就像你给别人发微信，发送成功了，但对方可能开了免打扰（DISABLE）没看到。
  if (pthread_cancel(tid) != 0) {
    perror("pthread_cancel");
  }
  printf("[主线程] 已发送取消请求\n");

  // ---------------- 4. 等待回收 ----------------
  pthread_join(tid, &res);

  // ---------------- 5. 检查结果 ----------------
  // 由于子线程忽略了取消请求并正常运行到了 return NULL;
  // 所以 res 的值应该是 NULL (即 0)，而不是 PTHREAD_CANCELED (-1)。
  if (res == PTHREAD_CANCELED) {
    printf("[主线程] Thread was canceled (子线程被取消了 - 不应发生)\n");
  } else {
    // 程序将执行到这里
    printf("[主线程] Thread was not canceled, exit code: %ld\n", (long)res);
  }

  return 0;
}
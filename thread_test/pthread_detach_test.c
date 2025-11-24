#include <pthread.h>  // 线程核心头文件
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // sleep 函数

/**
 * 子线程执行的函数
 */
void* task(void* arg) {
  printf("[子线程] Thread started (开始运行)\n");

  // 模拟耗时操作，睡眠 2 秒
  sleep(2);

  printf("[子线程] Thread finished (运行结束)\n");
  return NULL;
}

int main() {
  pthread_t tid;

  // 1. 创建线程
  // 默认情况下，新创建的线程是 joinable 的
  int ret = pthread_create(&tid, NULL, task, NULL);
  if (ret != 0) {
    perror("pthread_create");
    exit(1);
  }

  // 2. 线程分离 (Key Point)
  // pthread_detach
  // 告诉操作系统：“这个线程我不管了，它结束后请自动回收它的资源。”
  // 调用这行代码后，主线程就不能再对这个 tid 调用 pthread_join 了。
  // 如果不调用 detach 也不调用
  // join，子线程结束后会变成“僵尸线程”，造成内存泄漏。
  pthread_detach(tid);

  // 主线程继续工作，不会被阻塞
  printf("[主线程] Main thread continues (继续执行，不等待子线程)\n");

  // 3. 保持主线程存活 (Critical!)
  // 这是一个非常重要的概念：
  // 虽然线程被“分离”了，但这只是说资源的回收不需要主线程操心。
  // **并不代表** 子线程脱离了进程的生命周期。
  // 如果主线程（即进程的主入口）执行完毕 return 0 或 exit()，整个进程就会销毁。
  // 进程销毁时，其名下所有的线程（无论是否 detach）都会被强制立即杀死。

  // 子线程需要 2秒，如果主线程不睡 3秒，可能主线程刚打印完 ending 就退出了，
  // 导致子线程还没来得及打印 finished 就被杀掉了。
  sleep(3);
  printf("[主线程] Main thread ending (主线程结束，整个进程退出)\n");
  return 0;
}
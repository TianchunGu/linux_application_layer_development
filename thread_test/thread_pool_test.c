#include <glib.h>  // GLib 核心头文件
#include <stdio.h>
#include <stdlib.h>  // malloc, free
#include <unistd.h>  // sleep

/**
 * @brief 任务回调函数
 * 线程池中的线程会调用此函数来处理具体的工作。
 *
 * @param data      由 g_thread_pool_push 传入的任务数据 (这里是 int*)
 * @param user_data 创建线程池时传入的公共数据 (这里是 NULL)
 */
void task_func(gpointer data, gpointer user_data) {
  // 将泛型指针转回 int* 并取值
  int task_num = *(int*)data;

  // 释放内存
  // 注意：我们在 main 中为每个任务 malloc 了一块内存，
  // 必须在这里释放，否则会内存泄漏。
  free(data);

  printf("Executing task is %d... (Thread ID: %p)\n", task_num,
         (void*)pthread_self());

  // 模拟耗时任务 (1秒)
  sleep(1);

  printf("Task %d completed\n", task_num);
}

int main() {
  // ---------------- 1. 创建线程池 ----------------
  // GThreadPool* g_thread_pool_new (GFunc func, gpointer user_data, gint
  // max_threads, gboolean exclusive, GError **error); 参数解析：
  // 1. task_func:  任务执行函数。
  // 2. NULL:       传递给任务函数的 user_data (公共参数)，这里不需要。
  // 3. 5:          max_threads，表示池子中最多同时有 5 个线程在运行。
  // 4. TRUE:       exclusive (独占模式)。
  //                TRUE:  表示该线程池拥有自己独立的线程组，不与其他池共享。
  //                FALSE: 表示与其他非独占池共享全局线程资源。
  // 5. NULL:       用于接收错误的 GError 对象。
  GThreadPool* thread_pool = g_thread_pool_new(task_func, NULL, 5, TRUE, NULL);

  if (thread_pool == NULL) {
    fprintf(stderr, "Failed to create thread pool\n");
    return 1;
  }

  // ---------------- 2. 向线程池添加任务 ----------------
  printf("Pushing 10 tasks to the pool...\n");
  for (int i = 0; i < 10; i++) {
    // 动态分配内存存放参数
    // ⚠️ 重要：必须使用 malloc 分配独立的内存空间传给线程。
    // 如果直接传递 &i，由于主线程循环极快，子线程读取时 i
    // 的值可能已经变了（竞态条件）。
    int* tmp = malloc(sizeof(int));
    *tmp = i + 1;

    // g_thread_pool_push(pool, data, error)
    // 将数据推入队列。如果有空闲线程，立即执行；否则排队等待。
    g_thread_pool_push(thread_pool, tmp, NULL);
  }

  // ---------------- 3. 销毁线程池并等待 ----------------
  // g_thread_pool_free (pool, immediate, wait_)
  // 参数解析：
  // 2. FALSE (immediate):
  //    FALSE: 等待队列中所有排队的任务都执行完毕才销毁。
  //    TRUE:  立即销毁，丢弃队列中尚未开始的任务，只等待正在运行的任务。
  // 3. TRUE (wait_):
  //    TRUE:  阻塞当前主函数，直到线程池彻底关闭才返回。
  //    FALSE:
  //    不阻塞，主函数继续往下跑（通常会导致主程序退出，进而强制杀死子线程）。
  g_thread_pool_free(thread_pool, FALSE, TRUE);

  printf("All tasks completed\n");

  return 0;
}
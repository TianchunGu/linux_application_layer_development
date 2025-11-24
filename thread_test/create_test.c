#include <pthread.h>  // 包含 POSIX 线程库，用于创建和管理线程
#include <stdio.h>    // 标准输入输出库
#include <stdlib.h>   // 标准库，提供 malloc, free, exit 等函数
#include <unistd.h>   // 提供 sleep 函数

#define BUF_LEN 1024  // 定义缓冲区的大小常量

// 全局变量：共享缓冲区指针
// 因为是全局变量，所以 input_thread 和 output_thread 都能访问它
char* buf;

/**
 * @brief 输入线程（生产者）：读取标准输入的数据，放入缓存区
 *
 * @param argv 线程函数的参数，此处未使用，固定为NULL
 * @return void* 线程返回值，此处不需要，返回NULL
 */
void* input_thread(void* argv) {
  // i 是该线程的局部变量，维护写入位置的索引
  int i = 0;

  while (1) {
    // 1. 从标准输入（通常是键盘）读取一个字符
    // fgetc 会阻塞等待输入
    char c = fgetc(stdin);

    // 2. 简单的过滤逻辑：
    // 如果字符有效（非空）且不是换行符（回车），则写入缓冲区
    if (c && c != '\n') {
      buf[i++] = c;  // 写入字符后，索引自增

      // 3. 环形缓冲区逻辑（防止溢出）
      // 如果写到了缓冲区的末尾，这就回到开头（索引归零）
      // 注意：这里没有检查"数据是否被读取"，如果写得太快，会覆盖旧数据
      if (i >= BUF_LEN) {
        i = 0;
      }
    }
  }
  return NULL;
}

/**
 * @brief 输出线程（消费者）：从缓存区读取数据写到标准输出，每个字符换行
 *
 * @param argv 未使用
 * @return void* 未使用
 */
void* output_thread(void* argv) {
  // i 是该线程的局部变量，维护读取位置的索引
  // 注意：input_thread 和 output_thread 有各自独立的 i，互不干扰
  int i = 0;

  while (1) {
    // 1. 检查缓冲区当前位置是否有数据
    // 这里的逻辑假设：如果不为0，说明有新数据（因为读取后会置为0）
    if (buf[i]) {
      // 2. 从共享缓存区读取一个字节
      // 写入标准输出（屏幕）
      fputc(buf[i], stdout);
      // 额外输出一个换行符
      fputc('\n', stdout);

      // 3. 清理数据（消费完成）
      // 将已读取的位置置为 0，标记为空闲
      buf[i++] = 0;

      // 4. 环形缓冲区逻辑
      // 如果读到了末尾，回到开头
      if (i >= BUF_LEN) {
        i = 0;
      }
    } else {
      // 5. 如果当前位置没有数据（buf[i] == 0）
      // 线程睡眠 1 秒，避免空转消耗 CPU 资源（轮询等待）
      sleep(1);
    }
  }
  return NULL;
}

int main() {
  // 定义两个线程 ID 变量
  pthread_t pid_input;
  pthread_t pid_output;

  // 1. 在堆区动态分配 1024 字节的内存作为共享缓冲区
  buf = malloc(BUF_LEN);
  if (buf == NULL) {
    perror("malloc failed");
    return -1;
  }

  // 2. 初始化缓冲区
  // 将所有字节清零，确保 output_thread 不会读取到垃圾数据
  for (int i = 0; i < BUF_LEN; i++) {
    buf[i] = 0;
  }

  // 3. 创建读取线程 (Input Thread)
  // 参数1: 线程ID指针
  // 参数2: 线程属性 (NULL 表示默认属性)
  // 参数3: 线程运行的函数指针
  // 参数4: 传递给线程函数的参数 (NULL)
  if (pthread_create(&pid_input, NULL, input_thread, NULL) != 0) {
    perror("create input thread failed");
    return -1;
  }

  // 4. 创建写出线程 (Output Thread)
  if (pthread_create(&pid_output, NULL, output_thread, NULL) != 0) {
    perror("create output thread failed");
    return -1;
  }

  // 5. 等待线程结束 (Join)
  // pthread_join 会阻塞主线程，直到对应的线程执行结束。
  // 因为 input_thread 和 output_thread 内部都是 while(1) 死循环，
  // 所以主线程会一直卡在这里，程序不会退出。
  pthread_join(pid_input, NULL);
  pthread_join(pid_output, NULL);

  // 6. 释放资源
  // 实际上由于上面是死循环，这行代码永远不会执行到，
  // 但写上它是良好的编程习惯。
  free(buf);

  return 0;
}
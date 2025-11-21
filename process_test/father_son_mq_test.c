#include <fcntl.h>   // 定义 O_CREAT, O_RDWR 等标志
#include <mqueue.h>  // 消息队列核心头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  // 权限模式定义
#include <time.h>      // struct timespec, clock_gettime
#include <unistd.h>    // fork, sleep, close

int main(int argc, char const* argv[]) {
  // ---------------- 1. 定义消息队列属性 ----------------
  struct mq_attr attr;
  // 核心参数: 队列中最大允许存放的消息数量
  attr.mq_maxmsg = 10;
  // 核心参数: 每条消息的最大字节数
  attr.mq_msgsize = 100;
  // 以下参数在 mq_open 创建队列时会被忽略，仅用于 mq_getattr
  attr.mq_flags = 0;    // 0 表示阻塞模式
  attr.mq_curmsgs = 0;  // 当前消息数

  // 定义消息队列名称 (必须以 '/' 开头)
  char* mq_name = "/father_son_mq";

  // ---------------- 2. 创建或打开消息队列 ----------------
  // O_RDWR: 读写模式打开 (因为父进程要写，子进程要读)
  // O_CREAT: 如果不存在则创建
  // 0664: 权限设置 (所有者读写，组读写，其他人只读)
  mqd_t mqdes = mq_open(mq_name, O_RDWR | O_CREAT, 0664, &attr);

  if (mqdes == (mqd_t)-1) {
    perror("mq_open");
    exit(EXIT_FAILURE);
  }

  // ---------------- 3. 创建子进程 ----------------
  // fork 后，子进程会继承父进程打开的文件描述符 mqdes
  // 因此父子进程可以直接使用同一个描述符操作队列
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  // ---------------- 4. 子进程逻辑 (消费者) ----------------
  if (pid == 0) {
    char read_buf[100];
    struct timespec time_info;

    // 循环接收 10 次消息
    for (size_t i = 0; i < 10; i++) {
      // 每次接收前清空缓冲区，防止数据残留
      memset(read_buf, 0, 100);

      // 设置超时时间: 获取当前时间 + 15秒
      // clock_gettime(0, ...) 中的 0 等同于 CLOCK_REALTIME
      clock_gettime(CLOCK_REALTIME, &time_info);
      time_info.tv_sec += 15;

      // 接收消息
      // 如果队列为空，进程会阻塞，直到有消息到达或超过 15秒
      // 参数 NULL 表示不关心消息的优先级
      if (mq_timedreceive(mqdes, read_buf, 100, NULL, &time_info) == -1) {
        perror("mq_timedreceive");
      }

      printf("[子进程] 接收到数据: %s\n", read_buf);
    }

    // ---------------- 5. 父进程逻辑 (生产者) ----------------
  } else {
    char send_buf[100];
    struct timespec time_info;

    // 循环发送 10 次消息
    for (size_t i = 0; i < 10; i++) {
      // 清空发送缓冲区
      memset(send_buf, 0, 100);
      // 格式化字符串写入 buffer
      sprintf(send_buf, "父进程的第%d次发送消息", (int)(i + 1));

      // 设置超时时间: 当前时间 + 5秒
      clock_gettime(CLOCK_REALTIME, &time_info);
      time_info.tv_sec += 5;

      // 发送消息
      // 如果队列已满 (超过 mq_maxmsg 10条)，进程会阻塞，直到有空间或超时
      // 参数 0 表示默认优先级
      if (mq_timedsend(mqdes, send_buf, strlen(send_buf), 0, &time_info) ==
          -1) {
        perror("mq_timedsend");
      }

      printf("[父进程] 发送一条消息, 休眠1s...\n");

      // 休眠 1 秒，模拟生产数据的耗时
      // 这也给了子进程读取数据的时间
      sleep(1);
    }

    // 等待子进程退出 (防止子进程变成僵尸进程)
    // 虽然原代码没写 wait，但在父子协作中通常建议加上 wait(NULL);
  }

  // ---------------- 6. 资源清理 ----------------

  // 无论是父进程还是子进程，退出前都应关闭文件描述符
  // 这只是断开当前进程与队列的连接
  close(mqdes);

  // 只有父进程负责“物理删除”消息队列
  // 确保所有进程都不再使用后，从系统中移除该队列名
  if (pid > 0) {
    mq_unlink(mq_name);
    printf("[父进程] 消息队列已清理，程序结束。\n");
  }

  return 0;
}
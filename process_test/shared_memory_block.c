#include <fcntl.h>  // 定义 O_CREAT, O_RDWR 等标志
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>  // shm_open 函数的头文件
#include <sys/wait.h>
#include <unistd.h>  // getpid 函数

int main() {
  // 定义变量 (注意：share 和 pid 在此段代码中声明了但未使用)
  char* share;
  pid_t pid;

  // 准备存放共享内存名称的缓冲区
  char shmName[100] = {0};

  // 1. 构造唯一的共享内存名称
  // 格式："/letter" + "当前进程ID"
  // 例如：如果当前进程ID是 1234，名称就是 "/letter1234"
  // 注意：POSIX 共享内存名称通常要求以 '/' 开头
  sprintf(shmName, "/letter%d", getpid());

  // 打印生成的名称，方便你去 /dev/shm 目录下查找
  printf("shmName: %s\n", shmName);

  // 2. 创建共享内存对象
  // shm_open: 在内核中创建一个共享内存对象
  // 参数1: 名称
  // 参数2: O_CREAT (不存在则创建) | O_RDWR (读写模式)
  // 参数3: 0644 (权限设置：所有者读写，其他人只读)
  // 返回值: 成功返回文件描述符 fd，失败返回 -1
  int fd;
  fd = shm_open(shmName, O_CREAT | O_RDWR, 0644);

  if (fd < 0) {
    // 开启失败报错
    // 注意：perror会自动换行，字符串里其实不需要再加 '\n'
    perror("共享内存对象开启失败!\n");
    exit(EXIT_FAILURE);
  }

  // 3. 死循环 (重点)
  // 程序运行到这里会卡住，持续占用 CPU
  // 目的：保持进程存活，不关闭文件描述符，也不结束程序。
  // 这时你可以打开另一个终端窗口，输入 'ls -l /dev/shm'
  // 你应该能看到一个名为 letterXXXX 的文件，大小为 0 (因为还没 ftruncate)
  while (1)
    ;
  return 0;
}
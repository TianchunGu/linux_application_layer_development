#include <fcntl.h>  // 定义 O_CREAT, O_RDWR 等标志
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>  // 核心头文件：包含 mmap, shm_open, shm_unlink 等声明
#include <sys/wait.h>  // wait 函数
#include <unistd.h>    // fork, getpid, ftruncate, close, sleep

int main() {
  // 定义指向共享内存区域的指针
  char* share;
  pid_t pid;

  // 构造共享内存对象的名称
  // POSIX共享内存名称必须以 '/' 开头
  char shmName[100] = {0};
  sprintf(shmName, "/letter%d", getpid());

  // 1. 创建或打开共享内存对象 (Shared Memory Object)
  // shm_open 返回一个文件描述符，类似于 open()
  // O_CREAT: 如果不存在则创建
  // O_RDWR: 以读写方式打开
  // 0644: 设定权限 (文件所有者读写，组和其他人只读)
  int fd;
  fd = shm_open(shmName, O_CREAT | O_RDWR, 0644);
  if (fd < 0) {
    perror(
        "共享内存对象开启失败");  // perror会自动加冒号和换行，字符串里一般不用加\n
    exit(EXIT_FAILURE);
  }

  // 2. 调整共享内存大小
  // 新创建的共享内存对象大小默认为 0，必须调用 ftruncate 扩容
  // 这里将其扩充为 100 字节
  ftruncate(fd, 100);

  // 3. 将共享内存对象映射到进程的虚拟地址空间
  // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t
  // offset); NULL: 让内核自动选择映射的起始地址 100: 映射长度 PROT_READ |
  // PROT_WRITE: 内存区域可读可写 MAP_SHARED:
  // 关键标志！表示对内存的修改会同步到底层对象，且其他映射该对象的进程可见（实现通信）
  // fd: 共享内存的文件描述符
  // 0: 偏移量，从头开始映射
  share = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  // 错误检查：mmap 失败返回 MAP_FAILED (通常是 (void*)-1)，而不是 NULL
  if (share == MAP_FAILED) {
    perror("共享内存对象映射到内存失败");
    exit(EXIT_FAILURE);
  }

  // 4. 关闭文件描述符
  // 映射建立后，fd 就不再需要了，关闭它不会影响已经建立的内存映射
  // 这也是一种良好的编程习惯，避免资源泄漏
  close(fd);

  // 5. 创建子进程
  pid = fork();

  if (pid == 0) {
    // ---------------- 子进程执行区域 ----------------

    // 向共享内存写入数据
    // 因为 share 已经被映射到共享内存，直接操作指针即可，像操作普通数组一样
    strcpy(share, "你是个好人!\n");
    printf("新学员(子进程 %d)完成回信!\n", getpid());

    // 注意：子进程执行完这里后，会继续向下执行 main 函数剩余部分
    // 包括最后的 shm_unlink。通常建议子进程在这里显式 exit(0);
  } else {
    // ---------------- 父进程执行区域 ----------------

    // 简单的同步：等待1秒，确保子进程已经把数据写进去了
    // 在生产环境中，通常会配合信号量(semaphore)或互斥锁使用，而不是 sleep
    sleep(1);

    // 直接读取共享内存中的数据
    printf("老学员(父进程 %d)看到新学员%d回信的内容: %s", getpid(), pid, share);

    // 等待子进程终止，回收僵尸进程
    wait(NULL);

    // 6. 解除内存映射
    // 进程结束时会自动解除，但显式调用是个好习惯
    int ret = munmap(share, 100);
    if (ret == -1) {
      perror("munmap");
      exit(EXIT_FAILURE);
    }
  }

  // 7. 删除共享内存对象
  // shm_unlink 类似于文件系统的 rm，将该名称从系统中移除
  // 注意：这不会立即销毁内存内容，直到所有映射该内存的进程都解除了映射 (munmap)
  // 或退出了 由于父子进程都会执行到这里，shm_unlink 会被调用两次。
  // 第一次成功，第二次会失败(因为已经删除了)，但不影响程序运行。
  shm_unlink(shmName);

  return 0;
}
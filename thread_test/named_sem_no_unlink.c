#include <fcntl.h>      // O_CREAT
#include <semaphore.h>  // 有名信号量核心头文件
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  // 定义有名信号量的名称
  // Linux 下通常在 /dev/shm/ 目录下可以看到名为 sem.named_sem 的文件
  char* sem_name = "/named_sem";

  // ---------------- 1. 初始化有名信号量 ----------------
  // sem_open: 打开或创建信号量
  // O_CREAT: 不存在则创建
  // 0666: 权限
  // 0: 初始值。这是同步的关键。
  //    这意味着第一个调用 sem_wait 的进程（父进程）会被阻塞，
  //    直到其他进程（子进程）调用 sem_post。
  sem_t* sem = sem_open(sem_name, O_CREAT, 0666, 0);

  // 建议添加错误判断
  if (sem == SEM_FAILED) {
    perror("sem_open");
    return -1;
  }

  // ---------------- 2. 创建子进程 ----------------
  pid_t pid = fork();

  if (pid > 0) {
    // ---------------- 父进程逻辑 ----------------

    // P操作 (Wait)
    // 因为初始值为 0，父进程运行到这里会【立即阻塞】。
    // 它必须等待子进程“发令”才能继续。
    sem_wait(sem);

    printf("this is father (父进程: 收到信号，开始执行)\n");

    // 等待子进程彻底退出，回收资源，防止僵尸进程
    waitpid(pid, NULL, 0);

    // ---------------- 资源处理 ----------------

    // 释放引用 (Close)
    // 关闭当前进程对信号量的连接（类似关闭文件描述符）
    sem_close(sem);

    // ⚠️ 关键注释：关于 sem_unlink
    // sem_unlink 的作用是从系统中【彻底删除】该信号量文件。
    // 你这里把它注释掉了，意味着：
    // 1. 程序结束后，信号量依然存在于内核/文件系统中 (/dev/shm/sem.named_sem)。
    // 2. 它的值保持为最后的状态。
    // 3. 下次运行程序时，sem_open
    // 会直接打开这个已存在的信号量，而不是重新创建。

    // if(sem_unlink(sem_name) == -1) {
    //    perror("sem_unlink");
    // }

  } else if (pid == 0) {
    // ---------------- 子进程逻辑 ----------------

    // 模拟耗时操作
    sleep(1);

    printf("this is son (子进程: 先执行)\n");

    // V操作 (Post)
    // 信号量值 +1。
    // 唤醒正在阻塞的父进程。
    sem_post(sem);

    // 释放引用 (Close)
    // 子进程使用完毕，关闭连接
    sem_close(sem);

  } else {
    perror("fork");
  }

  return 0;
}
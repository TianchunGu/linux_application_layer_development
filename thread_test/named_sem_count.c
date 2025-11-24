#include <fcntl.h>      // O_CREAT
#include <semaphore.h>  // 有名信号量核心头文件: sem_open, sem_wait, sem_post...
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>  // 权限模式常量
#include <sys/types.h>
#include <sys/wait.h>  // waitpid
#include <unistd.h>    // fork, sleep

int main() {
  // 定义有名信号量的名称
  // 在 Linux 中，有名信号量通常在 /dev/shm/ 下以文件形式存在
  char* sem_name = "/named_sem";

  // ---------------- 1. 初始化有名信号量 ----------------
  // sem_open: 打开或创建一个有名信号量
  // 参数 O_CREAT: 如果不存在则创建
  // 参数 0666: 权限设置
  // 参数 0 (关键点!): 初始值设为 0。
  //       这意味着：第一个调用 sem_wait 的进程会被立即阻塞，
  //       直到有另一个进程调用 sem_post 增加信号量的值。
  //       这用于【同步/顺序控制】，而不是互斥锁。
  sem_t* sem = sem_open(sem_name, O_CREAT, 0666, 0);

  // 建议在实际开发中添加错误检查：
  // if (sem == SEM_FAILED) { perror("sem_open"); return -1; }

  // ---------------- 2. 创建子进程 ----------------
  pid_t pid = fork();

  if (pid > 0) {
    // ---------------- 父进程逻辑 ----------------

    // P操作 (Wait)
    // 因为初始值为 0，父进程运行到这里会【立即阻塞/挂起】。
    // 它必须等待子进程执行 sem_post 后，信号量变为 1，父进程才能继续。
    sem_wait(sem);

    // 只有当子进程唤醒父进程后，这句话才会打印
    printf("this is father (父进程)\n");

    // 等待子进程彻底退出，回收僵尸进程资源
    waitpid(pid, NULL, 0);

    // ---------------- 资源清理 ----------------

    // 释放当前进程对信号量的引用 (关闭句柄)
    sem_close(sem);

    // 从系统中彻底删除该有名信号量
    // 类似于文件系统的 rm 命令。
    // 如果不执行这一步，程序结束后该文件依然存在于 /dev/shm/ 中。
    // 通常由“最后一个”退出的进程或管理者进程来执行。
    if (sem_unlink(sem_name) == -1) {
      perror("sem_unlink");
    }

  } else if (pid == 0) {
    // ---------------- 子进程逻辑 ----------------

    // 模拟耗时操作
    // 即使子进程在这里睡了 1 秒，父进程因为被 sem_wait 卡住，
    // 依然无法抢先打印。这保证了顺序的绝对性。
    sleep(1);

    printf("this is son (子进程)\n");

    // V操作 (Post / Signal)
    // 信号量值 +1 (从 0 变为 1)。
    // 系统会检测是否有进程阻塞在该信号量上（即父进程），如果有，则唤醒它。
    sem_post(sem);

    // 释放当前进程对信号量的引用
    // 子进程只需 close，不需要 unlink (除非逻辑规定由子进程销毁)
    sem_close(sem);

  } else {
    perror("fork");
  }

  return 0;
}
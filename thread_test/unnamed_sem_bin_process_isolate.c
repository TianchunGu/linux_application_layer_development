#include <fcntl.h>      // 定义 O_CREAT, O_RDWR
#include <semaphore.h>  // 信号量
#include <stdio.h>
#include <sys/mman.h>  // 共享内存
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>  // waitpid
#include <unistd.h>

int main() {
  // 共享内存名称
  char* shm_value_name = "unnamed_sem_shm_value";

  // 1. 创建/打开共享内存对象 (用于存放 int 数据)
  int value_fd = shm_open(shm_value_name, O_CREAT | O_RDWR, 0666);

  // 2. 设置共享内存大小 (int大小)
  ftruncate(value_fd, sizeof(int));

  // 3. 映射共享内存
  // 注意：这里只映射了 int 变量，没有映射信号量
  int* value =
      mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, value_fd, 0);

  // ---------------- ❌ 致命错误区域 ----------------
  // sem 定义在 main 函数的栈上 (Stack)，是一个局部变量。
  // 虽然 fork() 后子进程会复制父进程的栈空间，
  // 但在现代操作系统中，fork 采用“写时复制”(Copy-On-Write)。
  // 父进程的 &sem 和子进程的 &sem 虽然虚拟地址可能相同，
  // 但它们指向的是完全独立的物理内存！
  sem_t sem;

  // 初始化信号量
  // 参数2 (pshared) = 1：表示意图在进程间共享。
  // 但是，因为 sem
  // 变量本身不在共享内存中，所以这个标志位虽然设了，实际上无法生效。
  sem_init(&sem, 1, 1);

  // 初始化共享变量
  *value = 0;

  // 4. 创建子进程
  int pid = fork();

  if (pid > 0) {
    // ---------------- 父进程逻辑 ----------------

    // 操作的是父进程栈上的 sem
    sem_wait(&sem);

    // 临界区
    int tmp = *value + 1;
    sleep(1);  // 模拟耗时，此时子进程会运行
    *value = tmp;

    // 解锁父进程栈上的 sem
    sem_post(&sem);

    // 等待子进程结束
    waitpid(pid, NULL, 0);
    printf("this is father, child finished\n");

    // 预期结果是 2，但因为同步失败，实际输出通常是 1
    printf("the final value is %d\n", *value);
  } else if (pid == 0) {
    // ---------------- 子进程逻辑 ----------------

    // 操作的是子进程栈上的 sem (是父进程的副本)
    // 因为父进程锁的是它自己的 sem，子进程的 sem 依然是初始状态(1)
    // 所以子进程不会被阻塞，直接进入临界区！
    sem_wait(&sem);

    // 临界区 (与父进程同时进入了，发生竞态条件)
    int tmp = *value + 1;
    sleep(1);
    *value = tmp;

    sem_post(&sem);

    // 子进程退出
  } else {
    perror("fork");
  }

  // 5. 资源清理

  // 销毁信号量
  if (pid > 0) {
    if (sem_destroy(&sem) == -1) {
      perror("sem_destory");
    }
  }

  // 解除映射
  if (munmap(value, sizeof(int)) == -1) {
    perror("munmap value");
  }

  // 关闭文件描述符
  if (close(value_fd) == -1) {
    perror("close value");
  }

  // 删除共享内存对象
  if (pid > 0) {
    if (shm_unlink(shm_value_name) == -1) {
      perror("father shm_unlink shm_value_name");
    }
  }

  return 0;
}
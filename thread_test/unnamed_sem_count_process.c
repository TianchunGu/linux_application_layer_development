#include <fcntl.h>  // O_CREAT, O_RDWR
#include <pthread.h>
#include <semaphore.h>  // sem_t, sem_init...
#include <stdio.h>
#include <sys/mman.h>  // shm_open, mmap
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>  // waitpid
#include <unistd.h>

int main() {
  // 定义共享内存名称
  char* shm_name = "unnamed_sem_shm";

  // 1. 创建/打开共享内存对象
  // O_CREAT: 不存在则创建
  // O_RDWR: 以读写方式打开
  // 0666: 权限设置
  int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);

  // 2. 调整共享内存对象的大小
  // 新创建的共享内存大小为0，必须设置为足以容纳一个 sem_t 结构体的大小
  ftruncate(fd, sizeof(sem_t));

  // 3. 将共享内存对象映射到进程的虚拟地址空间
  // PROT_READ | PROT_WRITE: 可读可写
  // MAP_SHARED:
  // 关键标志！表示修改对其他映射了该对象的进程可见（实现进程间通信） sem
  // 指针现在指向这块共享内存
  sem_t* sem =
      mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  // 4. 初始化信号量
  // 参数2 (1): pshared=1，表示信号量在进程间共享（放在共享内存中才有意义）
  // 参数3 (0): value=0，初始值为0。这是一种“同步”用法。
  //           意味着调用 sem_wait 的人必须等待别人调用 sem_post 后才能通过。
  sem_init(sem, 1, 0);

  // 5. 创建子进程
  int pid = fork();

  if (pid > 0) {
    // ---------------- 父进程逻辑 ----------------

    // P操作 (Wait)
    // 因为初始值是 0，父进程运行到这里会被立即阻塞。
    // 它必须等待子进程执行 sem_post 后才能继续。
    sem_wait(sem);

    // 只有子进程运行完打印后，父进程才能打印
    printf("this is father\n");

    // 等待子进程彻底退出，回收僵尸进程资源
    waitpid(pid, NULL, 0);
  } else if (pid == 0) {
    // ---------------- 子进程逻辑 ----------------

    // 模拟耗时操作
    // 即使子进程睡了 1 秒，父进程也因为卡在 sem_wait 处而无法抢先打印。
    sleep(1);

    printf("this is son\n");

    // V操作 (Post / Signal)
    // 信号量值 +1。
    // 此时唤醒正在阻塞的父进程。
    sem_post(sem);
  } else {
    perror("fork");
  }

  // ---------------- 资源清理 ----------------

  // 6. 销毁信号量
  // 只有父进程负责销毁，且必须在确保子进程不再使用后（waitpid之后）进行。
  if (pid > 0) {
    if (sem_destroy(sem) == -1) {
      perror("father sem_destroy");
    }
  }

  // 7. 解除内存映射
  // ⚠️ 注意：原代码此处有一个 Bug
  // munmap 的第二个参数应该是映射内存的大小。
  // sizeof(sem) 计算的是指针的大小 (8字节)，而不是结构体 sem_t 的大小
  // (通常32字节)。 正确写法应该是: munmap(sem, sizeof(sem_t));
  if (munmap(sem, sizeof(sem)) == -1) {
    perror("munmap");
  }

  // 8. 关闭文件描述符
  if (close(fd) == -1) {
    perror("close");
  }

  // 9. 删除共享内存对象
  // shm_unlink 相当于文件系统的 rm，从系统中移除该名称。
  // 只能调用一次，通常由父进程执行。
  if (pid > 0) {
    if (shm_unlink(shm_name) == -1) {
      perror("father shm_unlink");
    }
  }

  return 0;
}
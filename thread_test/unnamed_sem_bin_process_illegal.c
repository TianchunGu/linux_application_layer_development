#include <fcntl.h>      // 定义 O_CREAT, O_RDWR 等标志
#include <semaphore.h>  // 信号量相关函数: sem_t, sem_init, sem_wait, sem_post
#include <stdio.h>
#include <sys/mman.h>  // 内存映射函数: shm_open, mmap
#include <sys/stat.h>  // 权限模式常量
#include <sys/types.h>
#include <sys/wait.h>  // waitpid
#include <unistd.h>

int main() {
  // 定义共享内存对象的名称 (在 /dev/shm/ 下可见)
  char* shm_sem_name = "unnamed_sem_shm_sem";
  char* shm_value_name = "unnamed_sem_shm_value";

  // ---------------- 1. 创建/打开共享内存对象 ----------------
  // shm_open 返回文件描述符
  // O_CREAT: 不存在则创建
  // O_RDWR: 读写模式
  // 0666: 权限设置
  int sem_fd = shm_open(shm_sem_name, O_CREAT | O_RDWR, 0666);
  int value_fd = shm_open(shm_value_name, O_CREAT | O_RDWR, 0666);

  // ---------------- 2. 设置共享内存大小 ----------------
  // 新创建的对象大小默认为 0，必须扩展以容纳对应的数据结构
  ftruncate(sem_fd, sizeof(sem_t));  // 容纳一个信号量结构体
  ftruncate(value_fd, sizeof(int));  // 容纳一个整数

  // ---------------- 3. 内存映射 ----------------
  // 将内核中的共享内存对象映射到当前进程的虚拟地址空间
  // MAP_SHARED:
  // 关键标志！表示对内存的修改会同步到底层对象，从而被其他映射了该对象的进程看到。
  sem_t* sem =
      mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, sem_fd, 0);
  int* value =
      mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, value_fd, 0);

  // ---------------- 4. 初始化资源 (注意：此处有 bug) ----------------

  // 初始化无名信号量
  // 参数 1: 信号量指针
  // 参数 2 (pshared): 这里代码写的是 0。
  //        ❌ 错误：0 表示信号量仅在当前进程的【线程】间共享。
  //        ✅ 正确：对于 fork 出来的【进程】间同步，此处必须设为 1 (非零)。
  // 参数 3 (value): 初始值 1，模拟互斥锁。
  sem_init(sem, 0, 1);

  // 初始化共享变量
  *value = 0;

  // ---------------- 5. 创建子进程 ----------------
  int pid = fork();

  if (pid > 0) {
    // ---------------- 父进程逻辑 ----------------

    // P操作 (加锁)
    // 如果子进程持有了锁，父进程会在此阻塞
    sem_wait(sem);

    // --- 临界区开始 ---
    int tmp = *value + 1;
    sleep(1);  // 即使睡 1 秒，因为有锁，子进程也进不来
    *value = tmp;
    // --- 临界区结束 ---

    // V操作 (解锁)
    // 唤醒可能正在等待的子进程
    sem_post(sem);

    // 等待子进程退出 (回收僵尸进程)
    waitpid(pid, NULL, 0);

    printf("this is father, child finished\n");
    // 打印最终结果
    printf("the final value is %d\n", *value);
  } else if (pid == 0) {
    // ---------------- 子进程逻辑 ----------------

    // P操作 (加锁)
    sem_wait(sem);

    // --- 临界区开始 ---
    int tmp = *value + 1;
    sleep(1);
    *value = tmp;
    // --- 临界区结束 ---

    // V操作 (解锁)
    sem_post(sem);

    // 子进程结束后自动退出
  } else {
    perror("fork");
  }

  // ---------------- 6. 资源清理 ----------------

  // 父进程负责销毁信号量
  // 只有当确定没有进程在使用该信号量时才能销毁
  if (pid > 0) {
    if (sem_destroy(sem) == -1) {
      perror("sem_destory");
    }
  }

  // 解除内存映射
  // ❌ 潜在 Bug: munmap 的第二个参数应该是映射内存的实际大小。
  // sizeof(sem) 计算的是指针的大小 (通常是 8 字节)，而不是 sem_t 结构体的大小
  // (通常 32 字节)。 ✅ 建议修改为: munmap(sem, sizeof(sem_t));
  if (munmap(sem, sizeof(sem)) == -1) {
    perror("munmap sem");
  }

  // 这里同理，sizeof(value) 计算的是 int* 指针的大小，不过碰巧也是 int 的大小
  // (如果都是4字节的话)， 但严谨写法应该是 sizeof(int)
  if (munmap(value, sizeof(int)) == -1) {
    perror("munmap value");
  }

  // 关闭文件描述符 (不影响映射，也不删除对象)
  if (close(sem_fd) == -1) {
    perror("close sem");
  }

  if (close(value_fd) == -1) {
    perror("close value");
  }

  // 删除共享内存对象
  // shm_unlink 相当于文件系统的 rm，从系统中移除名称。
  // 这里只让父进程执行一次即可。
  if (pid > 0) {
    if (shm_unlink(shm_sem_name) == -1) {
      perror("father shm_unlink shm_sem_name");
    }

    if (shm_unlink(shm_value_name) == -1) {
      perror("father shm_unlink shm_value_name");
    }
  }

  return 0;
}
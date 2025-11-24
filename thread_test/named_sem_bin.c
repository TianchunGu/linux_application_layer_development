#include <fcntl.h>      // O_CREAT
#include <semaphore.h>  // 有名信号量: sem_open, sem_close, sem_unlink
#include <stdio.h>
#include <sys/mman.h>  // 共享内存: shm_open, mmap
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  // 定义名称
  // 注意：POSIX IPC 对象的名称通常要求以 '/' 开头
  char* sem_name = "/named_sem";
  char* shm_name = "/named_sem_shm";

  // ---------------- 1. 初始化有名信号量 ----------------
  // sem_open: 创建或打开一个有名信号量
  // 参数1: 名称
  // 参数2: O_CREAT (如果不存在则创建)
  // 参数3: 0666 (权限)
  // 参数4: 1 (初始值)。这里设为1，用作互斥锁。
  // 返回值: 成功返回 sem_t* 指针，失败返回 SEM_FAILED
  sem_t* sem = sem_open(sem_name, O_CREAT, 0666, 1);
  if (sem == SEM_FAILED) {
    perror("sem_open");
    return -1;
  }

  // ---------------- 2. 初始化共享内存 ----------------
  // 创建共享内存对象
  int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);

  // 调整大小 (必须步骤，否则大小为0)
  ftruncate(fd, sizeof(int));

  // 映射到进程虚拟地址空间
  // MAP_SHARED: 必须设置，保证父子进程共享同一块物理内存
  int* value =
      mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  // 初始化共享变量的值
  *value = 0;

  // ---------------- 3. 创建子进程 ----------------
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
  }

  // ---------------- 4. 临界区逻辑 (父子进程共用) ----------------
  // 无论是父进程还是子进程，执行到这里都会尝试申请锁

  // P操作 (加锁)
  // 因为 sem_open 时初始值为 1，第一个到达的进程会通过，将值减为 0。
  // 后到的进程会被阻塞在这里。
  sem_wait(sem);

  // --- 开始临界区 ---
  int tmp = *value + 1;

  // 模拟耗时操作
  // 即使在这里睡 1 秒，因为持有信号量，另一个进程也无法进入修改数据。
  sleep(1);

  *value = tmp;
  // --- 结束临界区 ---

  // V操作 (解锁)
  sem_post(sem);

  // ---------------- 5. 资源清理 (进程级) ----------------

  // sem_close: 关闭当前进程对信号量的引用 (类似 close 文件描述符)
  // 这并不会销毁信号量，只是断开连接。
  sem_close(sem);

  if (pid > 0) {
    // ---------------- 父进程特有逻辑 ----------------

    // 等待子进程结束，确保子进程已经完成了操作
    waitpid(pid, NULL, 0);

    // 打印结果
    // 由于有信号量保护，结果必然是 2
    printf("子进程执行结束，value = %d\n", *value);

    // sem_unlink: 从系统中彻底删除该信号量
    // 类似于文件系统的
    // rm。如果不执行这一步，程序结束后该信号量依然存在于内核中，
    // 下次运行程序时如果 O_CREAT 但没指定 O_EXCL，可能会直接使用旧的信号量值。
    sem_unlink(sem_name);
  }

  // ---------------- 6. 共享内存清理 ----------------

  // 解除映射
  munmap(value, sizeof(int));

  // 关闭文件描述符
  close(fd);

  // 删除共享内存对象 (仅需由一个进程执行)
  if (pid > 0) {
    if (shm_unlink(shm_name) == -1) {
      perror("shm_unlink");
    }
  }

  return 0;
}
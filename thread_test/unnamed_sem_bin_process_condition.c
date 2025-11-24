#include <fcntl.h>  // 定义 O_CREAT, O_RDWR 等
#include <stdio.h>
#include <sys/mman.h>  // 共享内存核心头文件: shm_open, mmap
#include <sys/stat.h>  // 权限模式常量
#include <sys/types.h>
#include <sys/wait.h>  // waitpid
#include <unistd.h>

int main() {
  // 定义共享内存对象的名称
  // 在 Linux 中，这通常会在 /dev/shm/ 目录下创建一个文件
  char* shm_value_name = "unnamed_sem_shm_value";

  // 1. 创建或打开共享内存对象 (Shared Memory Object)
  // shm_open 返回一个文件描述符
  // O_CREAT: 不存在则创建
  // O_RDWR: 读写模式打开
  // 0666: 设置权限
  int value_fd = shm_open(shm_value_name, O_CREAT | O_RDWR, 0666);

  // 2. 调整共享内存对象的大小
  // 新创建的对象大小为 0，必须调用 ftruncate 设置实际大小
  // 这里我们需要存放一个 int 类型的数据
  ftruncate(value_fd, sizeof(int));

  // 3. 将共享内存对象映射到进程的虚拟地址空间
  // mmap(addr, length, prot, flags, fd, offset)
  // NULL: 让内核自动选择地址
  // PROT_READ | PROT_WRITE: 可读可写
  // MAP_SHARED: 关键标志！表示修改对其他映射了该对象的进程可见（实现通信）
  int* value =
      mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, value_fd, 0);

  // 4. 初始化共享变量的值
  // 直接像操作普通指针一样操作共享内存
  *value = 0;

  // 5. 创建子进程
  // fork 之后，父子进程会有独立的内存空间（栈、堆等），
  // 但是！由于 value 指针指向的是 MAP_SHARED 的内存区域，
  // 所以父子进程看到的 value 指向的是同一块物理内存。
  int pid = fork();

  if (pid > 0) {
    // ---------------- 父进程逻辑 ----------------

    // 模拟非原子的“读-改-写”操作 (竞态条件源头)
    // 1. 读取: 父进程读到 0
    int tmp = *value + 1;

    // 2. 休眠: 主动让出 CPU。此时子进程大概率会运行，
    // 子进程也会读到 0 (因为父进程还没写回去)。
    sleep(1);

    // 3. 写入: 父进程醒来，写入 1
    *value = tmp;

    // 等待子进程执行完毕 (防止僵尸进程)
    waitpid(pid, NULL, 0);

    printf("this is father, child finished\n");
    // 打印最终结果
    // 由于发生了竞态条件，这里通常打印 1，而不是预期的 2
    printf("the final value is %d\n", *value);

  } else if (pid == 0) {
    // ---------------- 子进程逻辑 ----------------

    // 1. 读取: 子进程读到 0 (假设此时父进程正在 sleep)
    int tmp = *value + 1;

    // 2. 休眠: 子进程也睡一会
    sleep(1);

    // 3. 写入: 子进程写入 1 (覆盖了父进程可能已经写入的 1，或者被父进程覆盖)
    *value = tmp;

  } else {
    perror("fork");
  }

  // 6. 解除映射 & 关闭描述符
  // 无论父子进程，退出前都应该做清理工作
  // munmap 解除虚拟地址映射
  if (munmap(value, sizeof(int)) == -1) {
    perror("munmap value");
  }

  // close 关闭文件描述符 (并不删除共享内存对象本身)
  if (close(value_fd) == -1) {
    perror("close value");
  }

  // 7. 删除共享内存对象 (unlink)
  // shm_unlink 相当于文件系统的 rm 命令，从系统中移除该名称。
  // 必须确保只调用一次，通常由父进程或负责管理的进程执行。
  // 如果不调用这一步，程序结束后 /dev/shm/ 下依然会有这个文件，占用系统资源。
  if (pid > 0) {
    if (shm_unlink(shm_value_name) == -1) {
      perror("father shm_unlink shm_value_name");
    }
  }

  return 0;
}
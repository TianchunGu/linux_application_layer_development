#include <fcntl.h>      // O_CREAT, O_RDWR
#include <semaphore.h>  // sem_t, sem_init, sem_wait, sem_post
#include <stdio.h>
#include <sys/mman.h>  // shm_open, mmap
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>  // waitpid
#include <unistd.h>

int main() {
  // 定义两个共享内存对象的名称
  // 1. 用于存放信号量结构体
  char* shm_sem_name = "unnamed_sem_shm_sem";
  // 2. 用于存放共享的整数变量
  char* shm_value_name = "unnamed_sem_shm_value";

  // ---------------- 1. 创建共享内存对象 ----------------
  // shm_open 返回文件描述符
  int sem_fd = shm_open(shm_sem_name, O_CREAT | O_RDWR, 0666);
  int value_fd = shm_open(shm_value_name, O_CREAT | O_RDWR, 0666);

  // ---------------- 2. 设置共享内存大小 ----------------
  // 刚创建的对象大小为0，必须扩展以容纳对应的数据类型
  ftruncate(sem_fd, sizeof(sem_t));  // 容纳一个信号量结构
  ftruncate(value_fd, sizeof(int));  // 容纳一个整数

  // ---------------- 3. 内存映射 ----------------
  // 将内核中的共享内存映射到当前进程的虚拟地址空间
  // MAP_SHARED: 关键标志！只有这样，父子进程对内存的修改才是互通的。
  sem_t* sem =
      mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, sem_fd, 0);
  int* value =
      mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, value_fd, 0);

  // ---------------- 4. 初始化资源 ----------------

  // 初始化无名信号量
  // 参数1: 信号量指针
  // 参数2: pshared = 1 (Critical!)
  //        这里必须是 1 (非零)，表示该信号量是在多个进程间共享的。
  //        如果是 0，则只能在同一进程的线程间共享。
  // 参数3: value = 1 (初始值)，模拟互斥锁。
  sem_init(sem, 1, 1);

  // 初始化共享变量
  *value = 0;

  // ---------------- 5. 创建子进程 ----------------
  int pid = fork();

  if (pid > 0) {
    // ---------------- 父进程逻辑 ----------------

    // P操作 (加锁)
    // 如果子进程持有了锁，父进程会在这里阻塞等待
    sem_wait(sem);

    // --- 临界区开始 ---
    int tmp = *value + 1;
    sleep(1);  // 即使父进程在这里睡1秒，因为持有锁，子进程也进不来
    *value = tmp;
    // --- 临界区结束 ---

    // V操作 (解锁)
    // 唤醒子进程
    sem_post(sem);

    // 等待子进程退出
    // 确保子进程操作完成，且不再使用资源
    waitpid(pid, NULL, 0);

    printf("this is father, child finished\n");
    // 预期结果：2。因为父子进程串行执行了加法。
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

    // 子进程退出，会隐含地关闭它持有的文件描述符，但不会munmap
  } else {
    perror("fork");
  }

  // ---------------- 6. 资源清理 ----------------

  // 父进程负责销毁信号量
  // 只有当没有进程阻塞在该信号量上时，才能安全销毁
  if (pid > 0) {
    if (sem_destroy(sem) == -1) {
      perror("sem_destory");
    }
  }

  // 解除内存映射
  // ⚠️ 注意代码逻辑修正：原代码中 munmap(sem, sizeof(sem)) 是有风险的。
  // sizeof(sem) 是指针大小(8字节)，而 sizeof(sem_t)
  // 才是结构体大小(通常32字节)。 正确写法应为 munmap(sem, sizeof(sem_t));
  if (munmap(sem, sizeof(sem_t)) == -1) {
    perror("munmap sem");
  }

  if (munmap(value, sizeof(int)) == -1) {
    perror("munmap value");
  }

  // 关闭文件描述符
  if (close(sem_fd) == -1) {
    perror("close sem");
  }

  if (close(value_fd) == -1) {
    perror("close value");
  }

  // 删除共享内存对象 (只需由一个进程执行一次)
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
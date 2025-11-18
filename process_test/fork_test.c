#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  printf("海哥教老学员%d春暖花开!\n", getpid());
  pid_t pid = fork();
  if (pid < 0) {
    // 创建新进程失败
    printf("新学员加入失败\n");
    return 1;
  } else if (pid == 0) {
    // 这里往下的代码都是新的子进程的
    printf("新学员%d加入成功,他是老学员%d推荐的\n", getpid(), getppid());
  } else {
    // 这里往下继续运行父进程
    printf("老学员%d继续深造,他推荐了%d\n", getpid(), pid);
  }
  return 0;
}
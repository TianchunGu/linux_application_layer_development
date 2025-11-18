#include <stdio.h>
#include <stdlib.h>
int main() {
  /*
  根据传入的命令启动一个进程
  参数: 传入可执行的shell命令
  return: 成功返回0 不支持shell返回-1 失败返回非0
  */
  int result = system("ping -c 10 www.atguigu.com");
  if (result != 0) {
    printf("无法执行命令");
    return 1;
  }

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("参数不够,上不了二楼.\n");
    return 1;  // 当没有传入参数时，应返回非零值表示错误
  }
  printf("我是%s %d,我跟海哥上二楼啦!\n", argv[1], getpid());
  return 0;
}
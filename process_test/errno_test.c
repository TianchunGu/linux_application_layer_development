#include <errno.h>
#include <stdio.h>

int main() {
  fopen("/opt", "a+");

  printf("errno: %d\n", errno);
  perror("文件打开出问题");

  return 0;
}

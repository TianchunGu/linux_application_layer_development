#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  int fd;
  char* pipe_path = "/tmp/myfifo";

  // 打开有名管道用于读取
  fd = open(pipe_path, O_RDONLY);
  if (fd == -1) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }

  char read_buff[100];
  ssize_t read_num;

  while ((read_num = read(fd, read_buff, 100)) > 0) {
    write(STDOUT_FILENO, read_buff, read_num);
  }

  if (read_num < 0) {
    perror("read");
    printf("管道数据读取异常，退出");
    exit(EXIT_FAILURE);
  }

  printf("接收管道退出，进程终止\n");
  close(fd);

  return 0;
}

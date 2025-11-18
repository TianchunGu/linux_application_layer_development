#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const* argv[]) {
  int fd = open("io.txt", O_RDONLY);
  if (fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  char buffer[1024];  // 创建一个缓冲区来存放读取的数据
  ssize_t bytes_read;

  while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
    // 将读取的数据写入标准输出
    write(STDOUT_FILENO, buffer, bytes_read);
  }
  if (bytes_read == -1) {
    perror("read");
    close(fd);
    exit(EXIT_FAILURE);
  }
  close(fd);  // 使用完毕后关闭文件描述符
  return 0;
}

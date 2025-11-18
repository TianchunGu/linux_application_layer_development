#include <stdio.h>

int main() {
  // 打开文件
  FILE* ioFile = fopen("io.txt", "r");
  if (ioFile == NULL) {
    printf("不能读不存在的文件");
  }
  /*
  FILE *__stream: 需要读取的文件
  return： 读取的一个字节 到文件结尾或出错返回EOF
  int fgetc (FILE *__stream)
  */
  char c = fgetc(ioFile);
  while (c != EOF) {
    printf("%c", c);
    c = fgetc(ioFile);
  }
  int result = fclose(ioFile);
  if (result != 0) {
    printf("关闭文件失败");
    return 1;
  }
  return 0;
}

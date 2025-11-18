#include <stdio.h>

int main() {
  // 打开文件
  FILE* ioFile = fopen("io.txt", "r");
  if (ioFile == NULL) {
    printf("不能读不存在的文件");
  }
  /*
  char *__restrict __s: 接收读取的数据字符串
  int __n: 能够接收数据的长度
  FILE *__restrict __stream: 需要读取的文件
  return: 成功返回字符串 失败返回NULL(可以直接用于while)
  fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
  */
  char buffer[100];
  while (fgets(buffer, sizeof(buffer), ioFile)) {
    printf("%s", buffer);
  }
  int result = fclose(ioFile);
  if (result != 0) {
    printf("关闭文件失败");
    return 1;
  }
  return 0;
}

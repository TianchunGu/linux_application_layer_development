#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const* argv[]) {
  // malloc动态分配内存  也可以用 char ch[100]接收数据
  char* ch = malloc(100);
  // char ch1[100];

  /*
  stdin: 标准输入FILE *
  */
  fgets(ch, 100, stdin);

  printf("你好:%s", ch);
  /*
  stdout: 标准输出FILE * 写入这个文件流会将数据输出到控制台
  printf底层就是使用的这个
  */
  fputs(ch, stdout);

  /*
  stderr: 错误输出FILE * 一般用于输出错误日志
  */
  fputs(ch, stderr);

  return 0;
}

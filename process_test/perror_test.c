#include <stdio.h>

int main(int argc, char const* argv[]) {
  /**
   * 输出错误信息报告到系统的错误输出
   * char *__s: 自定义的错误信息前缀 会打印在输出的前面 中间补充": " 后面跟errno
   * 隐藏参数: errno 用于保存错误信息的全局变量
   * 系统调用和库函数出错会将信息存储到这里 void perror (const char *__s)
   */
  fopen("bucunzai.txt", "r");
  perror("这道题我不会做! ");
  return 0;
}

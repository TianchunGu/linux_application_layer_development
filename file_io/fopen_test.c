#include <stdio.h>

int main() {
  /* 打开文件
  char *__restrict __filename: 字符串表示要打开文件的路径和名称
  char *__restrict __modes: 字符串表示访问模式
      (1)"r": 只读模式 没有文件打开失败
      (2)"w": 只写模式 存在文件写入会清空文件,不存在文件则创建新文件
      (3)"a": 只追加写模式 不会覆盖原有内容 新内容写到末尾，如果文件不存在则创建
      (4)"r+": 读写模式 文件必须存在 写入是从头一个一个覆盖
      (5)"w+": 读写模式 可读取,写入同样会清空文件内容，不存在则创建新文件
      (6)"a+": 读写追加模式 可读取,写入从文件末尾开始，如果文件不存在则创建
  return: FILE * 结构体指针 表示一个文件
  FILE *fopen (const char *__restrict __filename,
          const char *__restrict __modes)
  */
  char* filename = "io.txt";
  FILE* ioFile = fopen(filename, "a+");
  if (ioFile == NULL) {
    printf("FAILED，a+不能打开不存在的文件\n");
  } else {
    printf("SUCCESS，a+能打开不存在的文件\n");
  }
}

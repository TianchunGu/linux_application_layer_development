#include <stdio.h>
int main() {
  /* 打开文件
  char *__restrict __filename: 字符串表示要打开文件的路径和名称
  char *__restrict __modes: 字符串表示访问模式
      (1)"r": 只读模式 没有文件打开失败
      (2)"w": 只写模式 存在文件写入会清空文件,不存在文件则创建新文件
      (3)"a": 只追加写模式 不会覆盖原有内容 新内容写到末尾
      (4)"r+": 读写模式 文件必须存在 写入是从头一个一个覆盖
      (5)"w+": 读写模式 可读取,写入同样会清空文件内容或创建新文件
      (6)"a+": 读写追加模式 可读取,写入从文件末尾开始
  return: FILE * 结构体指针 表示一个文件 出错返回NULL
  FILE *fopen (const char *__restrict __filename,
          const char *__restrict __modes)
  */
  char* filename = "io1.txt";
  FILE* ioFile = fopen(filename, "r");
  if (ioFile == NULL) {
    printf("r不能打开不存在的文件\n");
  } else {
    printf("r能打开不存在的文件\n");
  }

  /*
  FILE *__stream: 需要关闭的文件
  return: 成功返回0 失败返回EOF(负数) 通常失败会造成系统崩溃
  int fclose (FILE *__stream)
  */
  int result = fclose(ioFile);
  if (result != 0) {
    printf("关闭文件失败");
    return 1;
  }

  return 0;
}

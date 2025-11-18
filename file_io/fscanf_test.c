#include <stdio.h>

int main() {
  /*
  FILE *__restrict __stream: 读取的文件
  char *__restrict __format: 读取的匹配表达式
  ...: 变长参数列表 用于接收匹配的数据
  return: 成功返回参数的个数  失败返回0 报错或结束返回EOF
  int fscanf (FILE *__restrict __stream,  const char *__restrict __format, ...)
  */
  FILE* userFile = fopen("user.txt", "r");
  if (userFile == NULL) {
    printf("不能打开不存在的文件");
  }
  char name[50];
  int age;
  char wife[50];
  int scanfR;
  while (fscanf(userFile, "%s %d %s\n", name, &age, wife) != EOF) {
    printf("%s在%d岁爱上了%s\n", name, age, wife);
  }

  int result = fclose(userFile);
  if (result != 0) {
    printf("关闭文件失败");
    return 1;
  }
  return 0;
}

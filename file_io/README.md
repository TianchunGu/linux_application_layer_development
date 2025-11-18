<font style="color:#3b3b3b;">我们在</font>`<font style="color:#3b3b3b;">/home/atguigu</font>`<font style="color:#3b3b3b;">下新建目录</font>`<font style="color:#3b3b3b;">file_io</font>`<font style="color:#3b3b3b;">，本章的所有例程全部放到该目录下。</font>

## C标准I/O库函数回顾

### 打开/关闭文件

#### `fopen`

（1）新建`fopen_test.c`，写入以下内容。

```c
#include <stdio.h>

int main()
{
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
    char *filename = "io.txt";
    FILE *ioFile = fopen(filename, "a+");
    if (ioFile == NULL)
    {
        printf("FAILED，a+不能打开不存在的文件\n");
    }
    else
    {
        printf("SUCCESS，a+能打开不存在的文件\n");
    }
}

```

（2）新建`Makefile`，写入以下内容

```makefile
CC:=gcc

fopen_test: fopen_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

<font style="color:#3b3b3b;">说明：</font>

<font style="color:#3b3b3b;">①有时编译器不只是</font>`<font style="color:#3b3b3b;">gcc</font>`<font style="color:#3b3b3b;">，我们将编译器定义为变量CC，当切换编译器时只需要更改该变量的定义，而无须更改整个</font>`<font style="color:#3b3b3b;">Makefile</font>`<font style="color:#3b3b3b;">。</font>

<font style="color:#3b3b3b;">② </font>`<font style="color:#3b3b3b;">$@</font>`<font style="color:#3b3b3b;">相当于当前</font>`<font style="color:#3b3b3b;">target</font>`<font style="color:#3b3b3b;">目标文件的名称，此处为</font>`<font style="color:#3b3b3b;">fopen_test</font>`<font style="color:#3b3b3b;">。</font>

<font style="color:#3b3b3b;">③ </font>`<font style="color:#3b3b3b;">$^</font>`<font style="color:#3b3b3b;">相当于当前</font>`<font style="color:#3b3b3b;">target</font>`<font style="color:#3b3b3b;">所有依赖文件列表，此处为</font>`<font style="color:#3b3b3b;">fopen_test.c</font>`<font style="color:#3b3b3b;">。</font>

<font style="color:#3b3b3b;">④ </font>`<font style="color:#3b3b3b;">./$@</font>`<font style="color:#3b3b3b;">的作用是执行目标文件。</font>

<font style="color:#3b3b3b;">⑤ </font>`<font style="color:#3b3b3b;">rm ./$@</font>`<font style="color:#3b3b3b;">的作用是在执行完毕后删除目标文件，如果没有这个操作，当源文件</font>`<font style="color:#3b3b3b;">fopen_test.c</font>`<font style="color:#3b3b3b;">未更改时就无法重复执行，会提示：make：“fopen_test”已是最新。此处删除目标文件，使得我们在不更改源文件的情况下可以多次执行。</font>

<font style="color:#3b3b3b;">⑥所有命令前都添加了“-”符号以忽略错误，确保即便上面的命令执行失败，仍然会向下执行。这样做是为了在发生错误时，确保删除目标文件，使得再次执行相同</font>`<font style="color:#3b3b3b;">target</font>`<font style="color:#3b3b3b;">时不会提示：make：“fopen_test”已是最新，可以重新执行target下的命令。</font>

（3）<font style="color:#3b3b3b;">执行结果如下</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763284146688-f05ad912-522f-4641-9c76-03e33624a1a7.png)

#### fclose

（1）创建fclose_test.c文件，写入以下内容。

```cpp
#include <stdio.h>
int main()
{

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
    char *filename = "io1.txt";
    FILE *ioFile = fopen(filename,"r");
    if (ioFile == NULL)
    {
        printf("r不能打开不存在的文件\n");
    }else{
        printf("r能打开不存在的文件\n");
    }

    /*
    FILE *__stream: 需要关闭的文件
    return: 成功返回0 失败返回EOF(负数) 通常失败会造成系统崩溃
    int fclose (FILE *__stream)
    */
    int result = fclose(ioFile);
    if (result != 0)
    {
        printf("关闭文件失败");
        return 1;
    }

    return 0;
}

```

（2）Makefile中补充以下内容

```makefile
fclose_test: fclose_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）执行结果如下

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763357451905-ffe15978-4b90-4bd2-9204-71786bc31b45.png)

### 向文件中写入数据

#### fputc 函数

（1）创建fputc_test.c文件，写入以下内容

```cpp
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
    char* filename = "io.txt";
    FILE* ioFile = fopen(filename, "a+");
    if (ioFile == NULL) {
        printf("a+不能打开不存在的文件\n");
    } else {
        printf("a+能打开不存在的文件\n");
    }
    /*
  写入文件一个字符
  int __c: 写入的char按照AICII值写入 可提前声明一个char
  FILE *__stream: 要写入的文件,写在哪里取决于访问模式
  return: 成功返回char的值 失败返回EOF
  int fputc (int __c, FILE *__stream)
  */
    int putcR = fputc(97, ioFile);

    if (putcR == EOF) {
        printf("写入字符失败\n");
    } else {
        printf("写入字符成功:%c\n", putcR);
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

```

（2）在Makefile中补充以下内容

```makefile
fputc_test: fputc_test.c
 -$(CC) -o $@ $^
 -./$@
 -rm ./$@
```

（3）运行代码查看结果

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763440093215-1160eec7-89ae-4e32-bd6c-89fd2c30fe84.png)

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763440099210-a9fbc244-b7fd-4c0e-a7fb-01551d3c9e8b.png)

#### fputs 函数

（1）创建fputs_test.c文件，写入以下内容

```cpp
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
    char* filename = "io.txt";
    FILE* ioFile = fopen(filename, "a+");
    if (ioFile == NULL) {
        printf("a+不能打开不存在的文件\n");
    } else {
        printf("a+能打开不存在的文件\n");
    }
    /*
  写入文件一个字符串
  char *__restrict __s: 需要写入的字符串
  FILE *__restrict __stream: 要写入的文件,写在哪里取决于访问模式
  return: 成功返回非负整数(一般是0,1) 失败返回EOF
  int fputs (const char *__restrict __s, FILE *__restrict __stream)
  */
    int putsR = fputs(" love letter\n", ioFile);
    if (putsR == EOF) {
        printf("写入字符串失败\n");
    } else {
        printf("写入字符串成功:%d\n", putsR);
    }
    /*
  FILE *__stream: 需要关闭的文件
  return: 成功返回0 失败返回EOF 通常失败会造成系统崩溃
  int fclose (FILE *__stream)
  */
    int result = fclose(ioFile);
    if (result != 0) {
        printf("关闭文件失败");
        return 1;
    }

    return 0;
}

```

（2）在Makefile中写入以下内容

```makefile
fputs_test: fputs_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）运行代码查看结果

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763440261864-22a98800-cacc-4db9-a583-53a7788a8d02.png)

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763440268870-5d1379f9-ca26-44e0-8dc7-746254af719f.png)

#### fprintf 函数

（1）创建fprintf_test.c文件，写入以下内容

```cpp
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
    char* filename = "io.txt";
    FILE* ioFile = fopen(filename, "a+");
    if (ioFile == NULL) {
        printf("a+不能打开不存在的文件\n");
    } else {
        printf("a+能打开不存在的文件\n");
    }

    /*
  FILE *__restrict __stream: 要写入的文件,写在哪里取决于访问模式
  char *__restrict __fmt: 格式化字符串
  ...: 变长参数列表
  return: 成功返回正整数(写入字符总数不包含换行符) 失败返回EOF
  fprintf (FILE *__restrict __stream, const char *__restrict __fmt, ...)
  */
    char* name = "大海";
    int fprintfR = fprintf(ioFile,
    "哎呀,那边窗户透出了什么光?\n那是东方,而你则是太阳!"
    "\n升起吧,骄阳,去让忌妒的月黯然失色!\n\t\t%s",
    name);
    if (fprintfR == EOF) {
        printf("写入字符串失败");
    } else {
        printf("写入字符串成功:%d\n", fprintfR);
    }

    /*
  FILE *__stream: 需要关闭的文件
  return: 成功返回0 失败返回EOF(负数) 通常失败会造成系统崩溃
  int fclose (FILE *__stream)
  */
    int result = fclose(ioFile);
    if (result != 0) {
        printf("关闭文件失败");
        fprintf(stderr, "%s\n", filename);
        return 1;
    }

    return 0;
}

```

（2）在Makefile中写入以下内容

```makefile
fprintf_test: fprintf_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）运行查看结果

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763440577379-eda738b6-055a-4881-9577-f6ea90c8ae58.png)

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763440584243-713d1ea3-3db7-4dca-ac38-8c9846240d6c.png)

### 从文件中读取数据

#### fgetc 函数

（1）创建fgetc_test.c文件，写入以下内容

```cpp
#include <stdio.h>

int main()
{
    // 打开文件
    FILE *ioFile = fopen("io.txt","r");
    if (ioFile == NULL)
    {
        printf("不能读不存在的文件");
    }
    /*
    FILE *__stream: 需要读取的文件
    return： 读取的一个字节 到文件结尾或出错返回EOF
    int fgetc (FILE *__stream)
    */
    char c = fgetc(ioFile);
    while (c != EOF)
        {
            printf("%c",c);
            c = fgetc(ioFile);
        }
    int result = fclose(ioFile);
    if (result != 0)
    {
        printf("关闭文件失败");
        return 1;
    }
    return 0;
}

```

（2）在Makefile中补充以下内容

```makefile
fgetc_test: fgetc_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）运行代码查看结果

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763440771036-93d89603-4237-4178-89d7-8e55d982ba26.png)

#### fgets 函数

（1）创建fgets_test.c，写入以下内容

```cpp
#include <stdio.h>

int main()
{
    // 打开文件
    FILE *ioFile = fopen("io.txt","r");
    if (ioFile == NULL)
    {
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
    if (result != 0)
    {
        printf("关闭文件失败");
        return 1;
    }
    return 0;
}

```

（2）Makefile中补充以下内容

```makefile
fgets_test: fgets_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）运行代码，查看结果

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763440966035-eba55db8-7873-464a-9e9a-19260af38e68.png)

#### fscanf 函数

（1）新建文件user.txt，写入以下内容

```plain
罗密欧 18 朱丽叶
贾宝玉 14 薛宝钗
梁山伯 16 祝英台

海哥
```

（2）创建fscanf_test.c，写入以下内容

```cpp
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

```

（3）Makefile补充以下内容

```makefile
fscanf_test: fscanf_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（4）运行代码，查看结果

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763441154591-8cf5c0c1-634e-4f1f-84bd-22f2cd6a5973.png)

### 标准输入/输出/错误

<font style="color:#3b3b3b;">读写文件通常用于代码内部操作，如果想要和用户沟通交流，就需要使用标准输入、输出和错误了。</font>

（1）<font style="color:#3b3b3b;">创建文件stdin_out_err_test.c，写入以下内容</font>

```cpp
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    // malloc动态分配内存  也可以用 char ch[100]接收数据
    char *ch = malloc(100);
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

```

（2）Makefile中补充以下内容

```makefile
stdin_out_err_test: stdin_out_err_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）<font style="color:#3b3b3b;">测试</font>

<font style="color:#3b3b3b;">①运行代码</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763441295264-2afe4fa1-696b-40ec-b1ba-87a46b8583f6.png)

<font style="color:#3b3b3b;">②在控制台输入海哥，会出现对应的标准输出和错误输出：</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763441316703-4e1cf93c-0060-4316-b645-06778a4f1fa5.png)

## 系统调用

### 关于系统调用

<font style="color:#3b3b3b;">系统调用是操作系统内核提供给应用程序，使其可以间接访问硬件资源的接口，关于操作系统内核、应用程序等概念，我们会在第五章详细阐述。</font>

### 常见系统调用

#### open

<font style="color:#3b3b3b;">open()系统调用用于打开一个标准的文件描述符。</font>

```cpp
#include <unistd.h>
#include <fcntl.h>

/*
    const char *__path: 文件路径
    int __oflag: 用于指定打开文件的方式,可以是以下选项的组合:
        (1) O_RDONLY: 以只读方式打开文件 
        (2) O_WRONLY: 以只写方式打开文件 
        (3) O_RDWR: 以读写方式打开文件 
        (4) O_CREAT: 如果文件不存在,则创建一个新文件 
        (5) O_APPEND: 将所有写入操作追加到文件的末尾 
        (6) O_TRUNC: 如果文件存在并且以写入模式打开,则截断文件长度为0 
        还有其他标志,如O_EXCL（当与O_CREAT一起使用时,只有当文件不存在时才创建新文件）、O_SYNC（同步I/O）、O_NONBLOCK（非阻塞I/O）等 
    可选参数: mode -> 仅在使用了O_CREAT标志且文件尚不存在的情况下生效,用于指定新创建文件的权限位 权限位通常由三位八进制数字组成,分别代表文件所有者、同组用户和其他用户的读写执行权限
    return: (1) 成功时返回非负的文件描述符。
            (2) 失败时返回-1，并设置全局变量errno以指示错误原因。
*/
int open (const char *__path, int __oflag, ...);
```

#### read

<font style="color:#3b3b3b;">read()系统调用用于读取已经打开的文件描述符。</font>

```cpp
#include <unistd.h>

/*
    int __fd:一个整数,表示要从中读取数据的文件描述符
    void *__buf:一个指向缓冲区的指针,读取的数据将被存放到这个缓冲区中
    size_t __nbytes:一个size_t类型的整数,表示要读取的最大字节数 系统调用将尝试读取最多这么多字节的数据,但实际读取的字节数可能会少于请求的数量
    return: (1) 成功时,read()返回实际读取的字节数 这个值可能小于__nbytes,如果遇到了文件结尾（EOF）或者因为网络读取等原因提前结束读取 
            (2) 失败时,read()将返回-1
*/
ssize_t read (int __fd, void *__buf, size_t __nbytes);
```

##### ssize_t

<font style="color:#3b3b3b;">ssize_t相关的宏定义如下</font>

```cpp
typedef __ssize_t ssize_t;
__STD_TYPE __SSIZE_T_TYPE __ssize_t;
# define __STD_TYPE     typedef
#define __SSIZE_T_TYPE      __SWORD_TYPE
# define __SWORD_TYPE       long int
```

<font style="color:#3b3b3b;">ssize_t</font><font style="color:#3b3b3b;">是</font><font style="color:#3b3b3b;">__ssize_t</font><font style="color:#3b3b3b;">的别名，后者是</font><font style="color:#3b3b3b;">long int</font><font style="color:#3b3b3b;">的别名，</font><font style="color:#3b3b3b;">long</font><font style="color:#3b3b3b;">是</font><font style="color:#3b3b3b;">long int</font><font style="color:#3b3b3b;">的简写，因此，</font><font style="color:#3b3b3b;">ssize_t</font><font style="color:#3b3b3b;">实际上是</font><font style="color:#3b3b3b;">long</font><font style="color:#3b3b3b;">类型的别名。</font>

##### <font style="color:#3b3b3b;">size_t</font>

<font style="color:#3b3b3b;">相关定义如下</font>

```cpp
typedef __SIZE_TYPE__ size_t;
#define __SIZE_TYPE__ long unsigned int
```

<font style="color:#3b3b3b;">unsigned long是long unsigned int的简写，size_t实质上是unsigned long。</font>

#### write

<font style="color:#3b3b3b;">write()系统调用用于对打开的文件描述符写入内容。</font>

```cpp
#include <unistd.h>

/*
    int __fd:一个整数,表示要写入数据的文件描述符
    void *__buf:一个指向缓冲区的指针,写入的数据需要先存放到这个缓冲区中
    size_t __n:一个size_t类型的整数,表示要写入的字节数 write()函数会尝试写入__n个字节的数据,但实际写入的字节数可能会少于请求的数量
    return: (1) 成功时,write()返回实际写入的字节数 这个值可能小于__n,如果写入操作因故提前结束,例如: 磁盘满、网络阻塞等情况 
            (2) 失败时,write()将返回-1
*/
ssize_t write (int __fd, const void *__buf, size_t __n);
```

#### close

<font style="color:#3b3b3b;">close()系统调用用于在使用完成之后，关闭对文件描述符的引用。</font>

```cpp
#include <unistd.h>

/*
    int __fd:一个整数,表示要关闭的文件描述符
    return: (1) 成功关闭时 返回0
            (2) 失败时 返回-1
*/
int close (int __fd);
```

#### exit和_exit()

##### <font style="color:#3b3b3b;">系统调用_exit()</font>

<font style="color:#3b3b3b;">_exit()</font><font style="color:#3b3b3b;">是由</font><font style="color:#3b3b3b;">POSIX</font><font style="color:#3b3b3b;">标准定义的系统调用，用于立即终止一个进程，定义在</font><font style="color:#3b3b3b;">unistd.h</font><font style="color:#3b3b3b;">中。这个调用确保进程立即退出，不执行任何清理操作。</font>

<font style="color:#3b3b3b;">_exit()</font><font style="color:#3b3b3b;">在子进程终止时特别有用，这可以防止子进程的终止影响到父进程（比如，防止子进程意外地刷新了父进程未写入的输出缓冲区）。</font>

<font style="color:#3b3b3b;">_exit和_Exit功能一样。</font>

```cpp
#include <unistd.h>

/**
 * 立即终止当前进程，且不进行正常的清理操作，如关闭文件、释放内存等。这个函数通常在程序遇到严重错误需要立即退出时使用，或者在某些情况下希望避免清理工作时调用。
 * 
 * int status: 父进程可接收到的退出状态码 0表示成功 非0表示各种不同的错误
 */
void _exit(int status);
void _Exit (int __status) ;
```

##### 库函数exit()

<font style="color:#3b3b3b;">exit()函数是由C标准库提供的，定义在stdlib.h中。</font>

```cpp
#include <stdlib.h>

/**
 * 终止当前进程,但是在此之前会执行3种清理操作
 * (1) 调用所有通过atexit()注册的终止处理函数(自定义)
 * (2) 刷新所有标准I/O缓冲区(刷写缓存到文件)
 * (3) 关闭所有打开的标准I/O流(比如通过fopen打开的文件)
 * 
 * int status: 父进程可接收到的退出状态码 0表示成功 非0表示各种不同的错误
 */
void exit(int status);
```

##### 使用场景

<font style="color:#3b3b3b;">①</font><font style="color:#3b3b3b;">通常在父进程中使用</font><font style="color:#3b3b3b;">exit()</font><font style="color:#3b3b3b;">，以确保程序在退出前能执行清理操作，如关闭文件和刷新输出。</font>

<font style="color:#3b3b3b;">②在子进程中，特别是在fork()之后立即调用了一个执行操作（如exec()）但执行失败时，推荐使用_exit()或_Exit()来确保子进程的快速、干净地退出，避免执行标准的清理操作，这些操作可能会与父进程发生冲突或不必要的重复。</font>

### 综合案例

<font style="color:#3b3b3b;">使用标准的系统调用，来对第二章的文件进行简单的读写操作：</font>

（1）<font style="color:#3b3b3b;">创建文件system_call_test.c，写入以下内容</font>

```cpp
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{ 
    int fd = open("io.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    char buffer[1024]; // 创建一个缓冲区来存放读取的数据
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
    close(fd); // 使用完毕后关闭文件描述符
    return 0;
}

```

（2）Makefile中补充以下内容

```makefile
system_call_test: system_call_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）运行结果如下

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763445855689-3da9f913-2e8d-4cd1-84d4-971da0a115c7.png)

## 文件描述符

### 定义

<font style="color:#3b3b3b;">在Linux系统中，当我们打开或创建一个文件（或套接字）时，操作系统会提供一个文件描述符（File Descriptor，FD），这是一个非负整数，我们可以通过它来进行读写等操作。</font>

<font style="color:#3b3b3b;">然而，文件描述符本身只是操作系统为应用程序操作底层资源（如文件、套接字等）所提供的一个引用或“句柄”。</font>

<font style="color:#3b3b3b;">在Linux中，文件描述符0、1、2是有特殊含义的。 </font>

+ 0是标准输入（stdin）的文件描述符
+ 1是标准输出（stdout）的文件描述符
+ 2是标准错误（stderr）的文件描述符

### 文件描述符关联的数据结构

#### <font style="color:#3b3b3b;">struct file</font>

<font style="color:#3b3b3b;">每个文件描述符都关联到内核一个</font><font style="color:#3b3b3b;">struct file</font><font style="color:#3b3b3b;">类型的结构体数据，结构体定义位于</font><font style="color:#3b3b3b;">Linux</font><font style="color:#3b3b3b;">系统的</font><font style="color:#3b3b3b;">/usr/src/linux-hwe-6.5-headers-6.5.0-27/include/linux/fs.h</font><font style="color:#3b3b3b;">文件中，从</font><font style="color:#3b3b3b;">992</font><font style="color:#3b3b3b;">行开始。</font>

<font style="color:#3b3b3b;">该结构体的部分关键字段如下。</font>

```cpp
struct file {
    ...... 
atomic_long_t f_count;       // 引用计数，管理文件对象的生命周期
    struct mutex f_pos_lock;     // 保护文件位置的互斥锁
    loff_t f_pos;                // 当前文件位置（读写位置）
    ...... 
struct path f_path;          // 记录文件路径
    struct inode *f_inode;              // 指向与文件相关联的inode对象的指针，该对象用于维护文件元数据，如文件类型、访问权限等
    const struct file_operations *f_op; // 指向文件操作函数表的指针，定义了文件支持的操作，如读、写、锁定等
    ...... 
void *private_data;                  // 存储特定驱动或模块的私有数据
    ......
} __randomize_layout
    __attribute__((aligned(4)));

```

<font style="color:#3b3b3b;">这个数据结构记录了与文件相关的所有信息，其中比较关键的是</font><font style="color:#3b3b3b;">f_path</font><font style="color:#3b3b3b;">记录了文件的路径信息，</font><font style="color:#3b3b3b;">f_inode</font><font style="color:#3b3b3b;">，记录了文件的元数据。</font>

#### <font style="color:#3b3b3b;">struct path</font>

<font style="color:#3b3b3b;">结构体定义位于Linux系统的/usr/src/linux-hwe-6.5-headers-6.5.0-27/include/linux/path.h文件中，从第8行开始。</font>

```cpp
struct path {
    struct vfsmount *mnt;
    struct dentry *dentry;
} __randomize_layout;
```

<font style="color:#3b3b3b;">该结构体只有两个属性，这里不再通过定义展开。</font>

Ø  struct vfsmount：是虚拟文件系统挂载点的表示，存储有关挂载文件系统的信息。

Ø  struct dentry：目录项结构体，代表了文件系统中的一个目录项。目录项是文件系统中的一个实体，通常对应一个文件或目录的名字。通过这个类型的属性，可以定位文件位置。

#### <font style="color:#3b3b3b;">struct inode</font>

<font style="color:#3b3b3b;">结构体定义位于</font><font style="color:#3b3b3b;">Linux</font><font style="color:#3b3b3b;">系统的</font><font style="color:#3b3b3b;">/usr/src/linux-hwe-6.5-headers-6.5.0-27/include/linux/fs.h</font><font style="color:#3b3b3b;">文件中，从</font><font style="color:#3b3b3b;">639</font><font style="color:#3b3b3b;">行开始。</font>

<font style="color:#3b3b3b;">struct inode结构的部分定义如下。</font>

```cpp
struct inode {
    umode_t i_mode; // 文件类型和权限。这个字段指定了文件是普通文件、目录、字符设备、块设备等，以及它的访问权限（读、写、执行）。
    unsigned short i_opflags;
    kuid_t i_uid; // 文件的用户ID，决定了文件的拥有者。
    kgid_t i_gid; // 文件的组ID，决定了文件的拥有者组。
    unsigned int i_flags;
    ...... 
unsigned long i_ino; // inode编号，是文件系统中文件的唯一标识。
    ...... 
loff_t i_size;       // 文件大小
} __randomize_layout;
```

### 文件描述符表关联的数据结构

#### <font style="color:#3b3b3b;">打开的文件表数据结构</font>

<font style="color:#3b3b3b;">struct files_struct</font><font style="color:#3b3b3b;">是用来维护一个进程（下文介绍）中所有打开文件信息的。</font>

<font style="color:#3b3b3b;">结构体定义位于</font><font style="color:#3b3b3b;">/usr/src/linux-hwe-6.5-headers-6.5.0-27/include/linux/fdtable.h</font><font style="color:#3b3b3b;">文件中，从</font><font style="color:#3b3b3b;">49</font><font style="color:#3b3b3b;">行开始。</font>

<font style="color:#3b3b3b;">部分字段如下。</font>

```cpp
struct files_struct {
    ...... 
    struct fdtable __rcu *fdt;   // 指向当前使用的文件描述符表（fdtable）
    ...... 
    unsigned int next_fd;        // 存储下一个可用的最小文件描述符编号
    ...... 
    struct file __rcu *fd_array[NR_OPEN_DEFAULT]; // struct file指针的数组，大小固定，用于快速访问。
};
```

<font style="color:#3b3b3b;">fdt</font><font style="color:#3b3b3b;">维护了文件描述符表，其中记录了所有打开的文件描述符和</font><font style="color:#3b3b3b;">struct file</font><font style="color:#3b3b3b;">的对应关系。</font>

#### <font style="color:#3b3b3b;">打开文件描述符表</font>

<font style="color:#3b3b3b;">打开文件描述符表底层的数据结构是</font><font style="color:#3b3b3b;">struct fdtable</font><font style="color:#3b3b3b;">。</font>

<font style="color:#3b3b3b;">结构体定义位于/usr/src/linux-hwe-6.5-headers-6.5.0-27/include/linux/fdtable.h文件中，从27行开始。如下。</font>

```cpp
struct fdtable {
    unsigned int max_fds;   // 文件描述符数组的容量，即可用的最大文件描述符
    struct file __rcu **fd; // 指向struct file指针数组的指针
    unsigned long *close_on_exec;
    unsigned long *open_fds;
    unsigned long *full_fds_bits;
    struct rcu_head rcu;
};
```

#### <font style="color:#3b3b3b;">fd_array和fd</font>

<font style="color:#3b3b3b;">fd_array</font><font style="color:#3b3b3b;">是一个定长数组，用于存储进程最常用的</font><font style="color:#3b3b3b;">struct file</font><font style="color:#3b3b3b;">。</font>

<font style="color:#3b3b3b;">fd</font><font style="color:#3b3b3b;">是一个指针，可以指向任何大小的数组，其大小由</font><font style="color:#3b3b3b;">max_fds</font><font style="color:#3b3b3b;">字段控制。它可以根据需要动态扩展，以容纳更多的文件描述符。</font>

<font style="color:#3b3b3b;">当打开文件描述符的数量不多于</font><font style="color:#3b3b3b;">NR_OPEN_DEFAULT</font><font style="color:#3b3b3b;">时，</font><font style="color:#3b3b3b;">fd</font><font style="color:#3b3b3b;">指向的通常就是</font><font style="color:#3b3b3b;">fd_array</font><font style="color:#3b3b3b;">，当文件描述符的数量超过</font><font style="color:#3b3b3b;">NR_OPEN_DEFAULT</font><font style="color:#3b3b3b;">时，会发生动态扩容，会将</font><font style="color:#3b3b3b;">fd_array</font><font style="color:#3b3b3b;">的内容复制到扩容后的指针数组，</font><font style="color:#3b3b3b;">fd</font><font style="color:#3b3b3b;">指向扩容后的指针数组。这一过程是内核控制的。</font>

#### <font style="color:#3b3b3b;">文件描述符和fd或fd_array的关系</font>

<font style="color:#3b3b3b;">文件描述符是一个非负整数，其值实际上就是其关联的</font><font style="color:#3b3b3b;">struct file</font><font style="color:#3b3b3b;">在</font><font style="color:#3b3b3b;">fd</font><font style="color:#3b3b3b;">指向的数组或</font><font style="color:#3b3b3b;">fd_array</font><font style="color:#3b3b3b;">中的下标。</font>

### 文件描述符引用图解

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763445411132-b35812e1-d8c4-49b1-861e-1d95053e2226.png)

### 小结

<font style="color:#3b3b3b;">当我们执行open()等系统调用时，内核会创建一个新的struct file，这个数据结构记录了文件的元数据（文件类型、权限等）、文件路径、支持的操作等，然后分配文件描述符，将struct file维护在文件描述符表中，最后将文件描述符返回给应用程序。我们可以通过后者对文件执行它所支持的各种函数操作，而这些函数的函数指针都维护在struct file_operations数据结构中。文件描述符实质上是底层数据结构struct file的一个引用或者句柄，它为用户提供了操作底层文件的入口。</font>

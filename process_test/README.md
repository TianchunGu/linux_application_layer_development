## Linux进程简介

### 什么是进程

<font style="color:#3b3b3b;">顾名思义，进程（</font><font style="color:#3b3b3b;">Process</font><font style="color:#3b3b3b;">）是正在运行的程序，是操作系统进行资源分配和调度的基本单位。程序是存储在硬盘或内存的一段二进制序列，是静态的，而进程是动态的。进程包括代码、数据以及分配给它的其他系统资源（如文件描述符、网络连接等）。</font>

<font style="color:#3b3b3b;">我们打开的</font><font style="color:#3b3b3b;">VMWare</font><font style="color:#3b3b3b;">、开启的浏览器都对应操作系统的一个进程。</font>

<font style="color:#3b3b3b;">在/home/atguigu下创建process_test目录，本章的所有文件都会放在该目录下。</font>

### 使用system函数生成子进程

<font style="color:#3b3b3b;">system函数是标准库中执行shell指令的函数，可以使用man 3 system命令查看其声明。</font>

（1）创建system_test.c，写入以下内容

```cpp
#include <stdio.h>
#include <stdlib.h>
int main() {
    /*
  根据传入的命令启动一个进程
  参数: 传入可执行的shell命令
  return: 成功返回0 不支持shell返回-1 失败返回非0
  */
    int result = system("ping -c 10 www.atguigu.com");
    if (result != 0) {
        printf("无法执行命令");
        return 1;
    }

    return 0;
}

```

<font style="color:#3b3b3b;">system中的shell命令表示向www.atguigu.com发送10个ICMP回声请求。-c 10是用来控制ICMP请求次数的，如果不加该参数，ping命令会无限期发送直至用户手动终止。</font>

（2）创建Makefile，写入以下内容。

```makefile
CC:=gcc

system_test: system_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

> **<font style="color:red;">要注意，Makefile更改完成后必须保存，否则make执行的还是之前的内容。</font>**
>

（3）ps -ef查看子进程

<font style="color:#3b3b3b;">①运行程序，system会帮助我们启动一个shell子进程。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763465854322-39955d35-fbb1-4482-a412-0265bdad765d.png)

<font style="color:#3b3b3b;">②</font><font style="color:#3b3b3b;">在子进程执行过程中，开启一个终端执行</font><font style="color:#3b3b3b;">ps -ef</font><font style="color:#3b3b3b;">指令查看所有进程，可以看到下面的内容：</font>

<font style="color:#3b3b3b;">ps -ef展示的进程是按照进程的pid升序排列的，通常，创建越晚的进程pid越大。因此，我们从下往上找，很快就可以定位刚刚启动的进程。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763465869275-11f878ea-2cdc-4f57-85a7-dbdbe98d4419.png)

<font style="color:#3b3b3b;">test程序的pid为6176，它启动了子进程6185（sh -c ping -c 10 www.atguigu.com），这个进程又启动了子进程6186（ping -c 10 </font>[<font style="color:#3b3b3b;">www.atguigu.com</font>](http://www.baidu.com)<font style="color:#3b3b3b;">）</font>

## 进程处理相关系统调用

<font style="color:#3b3b3b;">system函数用到的系统调用为fork、execve和waitpid，下面我们就来详细介绍以下这几个系统调用。</font>

### C标准中的main函数声明

<font style="color:#3b3b3b;">C</font><font style="color:#3b3b3b;">语言标准提供了以下两种</font><font style="color:#3b3b3b;">main</font><font style="color:#3b3b3b;">函数声明。</font>

#### <font style="color:#3b3b3b;">无参数形式</font>

```cpp
int main(void);
```

<font style="color:#3b3b3b;">或</font>

```cpp
int main();
```

<font style="color:#3b3b3b;">这两种形式都表示</font><font style="color:#3b3b3b;">main</font><font style="color:#3b3b3b;">函数不接收命令行参数。在</font><font style="color:#3b3b3b;">C99</font><font style="color:#3b3b3b;">标准之前，</font><font style="color:#3b3b3b;">main</font><font style="color:#3b3b3b;">函数没有参数的形式被写为</font><font style="color:#3b3b3b;">int main()</font><font style="color:#3b3b3b;">，这在某些情况下可能导致与</font><font style="color:#3b3b3b;">int main(void)</font><font style="color:#3b3b3b;">行为不完全相同的问题，因为</font><font style="color:#3b3b3b;">int main()</font><font style="color:#3b3b3b;">在老式的</font><font style="color:#3b3b3b;">C</font><font style="color:#3b3b3b;">语言标准中不明确指出函数是否接受参数。从</font><font style="color:#3b3b3b;">C99</font><font style="color:#3b3b3b;">标准开始，推荐使用</font><font style="color:#3b3b3b;">int main(void)</font><font style="color:#3b3b3b;">明确指明</font><font style="color:#3b3b3b;">main</font><font style="color:#3b3b3b;">函数不接受任何参数，以提高代码的可读性和一致性。</font>

#### <font style="color:#3b3b3b;">有参数形式</font>

```cpp
int main(int argc, char *argv[]);
```

+ argc：传递给程序的命令行参数的数量
+ argv：指向字符串数组的指针，存储了命令行参数
  + argv[0]通常是程序的名称
  + argv[1]到argv[argc-1]是实际的命令行参数

### fork

#### 相关系统调用及数据类型

##### <font style="color:#3b3b3b;">pid_t</font>

<font style="color:#3b3b3b;">关于返回值类型pid_t：这个类型定义在头文件/usr/include/x86-64_64-linux-gnu/sys/types.h中，定义如下。</font>

```cpp
// 函数定义
__pid_t fork (void)
// 类型定义
typedef __pid_t pid_t;
```

<font style="color:#3b3b3b;">可以看到，pid_t是__pid_t的别名，后者定义在/usr/include/x86-64_64-linux-gnu/bits/types.h中，相关宏定义如下。</font>

```cpp
__STD_TYPE __PID_T_TYPE __pid_t;
#define __PID_T_TYPE        __S32_TYPE
#define __S32_TYPE      int
#define __STD_TYPE     typedef
```

<font style="color:#3b3b3b;">__STD_TYPE预处理后被替换为typedef，__PID_T_TYPE预处理后被替换为int，因此，__pid_t实际上是这样定义的</font>

```cpp
typedef int __pid_t;
```

<font style="color:#3b3b3b;">它是</font><font style="color:#3b3b3b;">int</font><font style="color:#3b3b3b;">的别名，所以</font><font style="color:#3b3b3b;">pid_t</font><font style="color:#3b3b3b;">也是</font><font style="color:#3b3b3b;">int</font><font style="color:#3b3b3b;">的别名。</font>

##### <font style="color:#3b3b3b;">fork()</font>

```cpp
/**
 * @brief 创建一个子进程(相当于复制,包括内存空间)
 * void: 不需要填写参数
 *  
 * @return pid_t pid_t(int值) 
 *  (1) 在父进程中 返回子进程的PID
 *  (2) 在子进程中 返回0
 *  (3) 发生错误 返回-1
 */
pid_t fork(void);
```

##### getpid()

```cpp
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief 返回调用进程的PID
 * 
 * @return pid_t 不会失败，必然返回进程PID
 */
pid_t getpid(void);
```

##### getppid()

```cpp
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief 返回调用进程父进程的PID
 * 
 * @return pid_t 不会失败，必然返回父进程PID，即PPID
 */
pid_t getppid(void);
```

#### 测试例程

（1）创建文件fork_test.c，写入以下内容

```cpp
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    printf("海哥教老学员%d春暖花开!\n",getpid());
    pid_t pid =  fork();
    if (pid < 0)
    {
        // 创建新进程失败
        printf("新学员加入失败\n");
        return 1;
    }else if (pid == 0){
        // 这里往下的代码都是新的子进程的
        printf("新学员%d加入成功,他是老学员%d推荐的\n",getpid(),getppid());
    }else{
        // 这里往下继续运行父进程
        printf("老学员%d继续深造,他推荐了%d\n",getpid(),pid);
    }
    return 0;
}
```

（2）Makefile写入以下内容

```makefile
fork_test: fork_test.c
  -$(CC) -o $@ $^
  -./$@
  -rm ./$@
```

（3）执行

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763466359356-8fc1f3f6-41d0-474d-8c49-0938ccc30729.png)

（4）执行过程分析

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763466437083-21f7afc3-f893-4149-ba38-2b3f255bedcf.png)

### 文件描述符的引用计数和close()

#### 案例

##### <font style="color:#3b3b3b;">sleep()</font>

<font style="color:#3b3b3b;">函数原型</font>

```cpp
#include <unistd.h>

/**
 * 整个进程睡眠指定秒数,如果在sleep()期间进程接收到信号,且信号有相应的处理函数,则sleep()可能提前结束,并返回剩余未休眠的时间
 * 
 * int seconds: 暂停的秒数
 * return: unsigned int 如果sleep()函数正常执行且休眠时间结束,则返回0;如果sleep()由于接收到信号而被提前唤醒,则返回剩余的未休眠秒数
 */
unsigned int sleep(unsigned int seconds);
```

##### <font style="color:#3b3b3b;">测试例程</font>

<font style="color:#3b3b3b;">当我们执行</font><font style="color:#3b3b3b;">fork()</font><font style="color:#3b3b3b;">系统调用时，子进程会复制父进程的资源，包括文件描述符。思考下面的案例。</font>

<font style="color:#3b3b3b;">创建fork_fd_test.c，写入以下内容。</font>

```cpp
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    // fork之前
    // 打开一个文件
    int fd = open("io.txt",O_CREAT | O_WRONLY | O_APPEND ,0644);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    char buffer[1024];//缓冲区存放写出的数据
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }else if (pid == 0){
        // 子进程代码
        strcpy(buffer,"这是子进程写入的数据!\n");
    }else {
        // 父进程代码
        sleep(1);
        strcpy(buffer,"这是父进程写入的数据!\n");
    }
    // 父子进程都要执行的代码
    ssize_t bytes_write = write(fd,buffer,strlen(buffer));
   
    if (bytes_write == -1)
    {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("写入数据成功\n");
    // 使用完毕之后关闭
    close(fd);
    if (pid == 0)
    {
        printf("子进程写入完毕,并释放文件描述符\n");
    }else{
        printf("父进程写入完毕,并释放文件描述符\n");
    }
    return 0;
}

```

<font style="color:#3b3b3b;">我们通过open()以写入追加的模式打开了io.txt文件，如果这个文件不存在则创建。查看VScode工作目录/home/atguigu/process_test，不存在io.txt。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763475299598-409b5e96-20fb-447f-b312-1524e5e796b6.png)

<font style="color:#3b3b3b;">上述程序的逻辑是：打开</font><font style="color:#3b3b3b;">io.txt</font><font style="color:#3b3b3b;">文件，获得文件描述符后，执行</font><font style="color:#3b3b3b;">fork()</font><font style="color:#3b3b3b;">创建子进程。分别在父子进程中向文件追加写入，并在写入完成后关闭。为了区分父子进程的操作，我们在父进程中通过</font><font style="color:#3b3b3b;">sleep()</font><font style="color:#3b3b3b;">休眠</font><font style="color:#3b3b3b;">1s</font><font style="color:#3b3b3b;">。</font>

（1）<font style="color:#3b3b3b;">Makefile</font>

<font style="color:#3b3b3b;">Makefile中写入以下内容。</font>

```makefile
fork_fd_test: fork_fd_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

#### 测试

（1）执行

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763475377491-e757135e-abbd-46c3-9f1a-bbc619619449.png)

（2）查看io.txt

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763475402750-49896e59-abea-4983-b6d0-28bf263a768c.png)

（3）<font style="color:#3b3b3b;">分析（</font>[看视频理解](https://www.bilibili.com/video/BV1DJ4m1M77z?p=33)<font style="color:#3b3b3b;">）</font>

<font style="color:#3b3b3b;">子进程复制了父进程的文件描述符</font><font style="color:#3b3b3b;">fd</font><font style="color:#3b3b3b;">，二者指向的应是同一个底层文件描述（</font><font style="color:#3b3b3b;">struct file</font><font style="color:#3b3b3b;">结构体）。我们发现，子进程通过</font><font style="color:#3b3b3b;">close()</font><font style="color:#3b3b3b;">释放文件描述符之后，父进程对于相同的文件描述符执行</font><font style="color:#3b3b3b;">write()</font><font style="color:#3b3b3b;">操作仍然成功了。这是为什么？</font>

<font style="color:#3b3b3b;">struct file结构体中有一个属性为引用计数，记录的是与当前struct file绑定的文件描述符数量。close()系统调用的作用是将当前进程中的文件描述符和对应的struct file结构体解绑，使得引用计数减一。如果close()执行之后，引用计数变为0，则会释放struct file相关的所有资源。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763475436155-3bd0e15e-891b-436b-a93a-bcc216103b06.png)

### execve

#### execve单独测试

##### <font style="color:#3b3b3b;">创建erlou.c </font>

<font style="color:#3b3b3b;">exec系列函数可以在同一个进程中跳转执行另外一个程序，我们先准备一个可执行程序erlou，通过编译erlou.c获得。</font>

```cpp
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("参数不够,上不了二楼.\n");
        return 1; // 当没有传入参数时，应返回非零值表示错误
    }
    printf("我是%s %d,我跟海哥上二楼啦!\n", argv[1], getpid());
    return 0;
}
```

##### 创建execve_test.c，写入以下内容

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    /*exec系列函数  父进程跳转进入一个新进程
    推荐使用execve
    char *__path: 需要执行程序的完整路径名
    char *const __argv[]: 指向字符串数组的指针 需要传入多个参数
        (1) 需要执行的程序命令(同*__path)
        (2) 执行程序需要传入的参数
        (3) 最后一个参数必须是NULL
    char *const __envp[]: 指向字符串数组的指针 需要传入多个环境变量参数
        (1) 环境变量参数 固定格式 key=value
        (2) 最后一个参数必须是NULL
    return: 成功就回不来了 下面的代码都没有意义
            失败返回-1
    int execve (const char *__path, char *const __argv[], char *const __envp[]) 
    */
    char *name = "banzhang";
    printf("我是%s %d,我现在在一楼\n",name,getpid());
    // 参数没填写够也能完成跳转,错误信息会在新程序中
    // char *argv[] = {"/home/atguigu/process_test/erlou",NULL};
    char *args[] = {"/home/atguigu/process_test/erlou",name,NULL};
    // 环境变量可以不传
    // char *envp[] = {NULL};
    char *envs[] = {"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin",NULL};
    int re = execve(argv[0],args,envs);
    if (re == -1)
    {
        printf("你没机会上二楼\n");
        return -1;
    }
    
    return 0;
}

```

##### Makefile

```makefile
erlou: erlou.c
    -$(CC) -o $@ $^

execve_test: execve_test.c erlou
    -$(CC) -o $@ $<
    -./$@
    -rm ./$@ ./$(word 2, $^)
```

<font style="color:#3b3b3b;">①</font><font style="color:#3b3b3b;"> $<</font><font style="color:#3b3b3b;">：取依赖列表的第一个文件，此处是</font><font style="color:#3b3b3b;">execve_test.c</font><font style="color:#3b3b3b;">。等价于</font><font style="color:#3b3b3b;">$(firstword $^)</font><font style="color:#3b3b3b;">。</font>

<font style="color:#3b3b3b;">②</font><font style="color:#3b3b3b;"> $(word 2, $^)</font><font style="color:#3b3b3b;">：取依赖列表的第二个文件，此处是</font><font style="color:#3b3b3b;">erlou</font><font style="color:#3b3b3b;">，这里也可以用</font><font style="color:#3b3b3b;">$(lastword $^)</font><font style="color:#3b3b3b;">表示取依赖列表的最后一个文件，也是</font><font style="color:#3b3b3b;">erlou</font><font style="color:#3b3b3b;">。</font>

##### <font style="color:#3b3b3b;">运行测试</font>

<font style="color:#3b3b3b;">点击运行execve_test即可，结果如下。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763475709746-46010baa-9f03-4564-b318-3a36df4205ae.png)

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763475721818-34de0573-ea83-49fe-af4a-510f929c9ca6.png)

#### execve+fork

##### <font style="color:#3b3b3b;">测试例程</font>

<font style="color:#3b3b3b;">可以</font><font style="color:#3b3b3b;">fork</font><font style="color:#3b3b3b;">和</font><font style="color:#3b3b3b;">exec</font><font style="color:#3b3b3b;">共同使用，实现场景老学员推荐新学员在二楼学习，自己保持不变。</font>

<font style="color:#3b3b3b;">创建fork_execve_test.c，写入以下内容。</font>

```cpp
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char const *argv[])
{
    char *name="老学员";
    printf("%s%d在一楼精进\n",name,getpid());
    __pid_t pid = fork();
    if (pid == -1)
    {
        printf("邀请新学员失败!\n");
    }else if (pid == 0)
    {
        // 新学员在这里
        char *newName = "ergou";
        char *args[] = {"/home/atguigu/process_test/erlou",newName,NULL};
        char *envs[] = {"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin",NULL};
        int re = execve(argv[0],args,envs);
        if (re == -1)
        {
            printf("新学员上二楼失败\n");
            return 1;
        }
        
    }else{
        // 老学员在这里
        printf("老学员%d邀请完%d之后还是在一楼学习\n",getpid(),pid);
        
    }
    return 0;
}

```

##### Makefile

```makefile
fork_execve_test: fork_execve_test.c erlou
    -$(CC) -o $@ $(firstword $^)
    -./$@
    -rm ./$@ ./$(lastword $^)
```

##### 运行复合案例

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763475807063-6ed32385-1827-4858-82dc-8de1e7dad720.png)

##### 执行过程图解

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763475864532-fcf0efdc-924f-4396-aa3a-0610050039ea.png)

### waitpid
>
> [视频讲解](https://www.bilibili.com/video/BV1DJ4m1M77z?p=38)
>

<font style="color:#3b3b3b;">Linux中父进程除了可以启动子进程，还要负责回收子进程的状态。如果子进程结束后父进程没有正常回收，那么子进程就会变成一个僵尸进程——即程序执行完成，但是进程没有完全结束，其内核中PCB结构体（下文介绍）没有释放。在上面的例子中，父进程在子进程结束前就结束了，那么其子进程的回收工作就交给了父进程的父进程的父进程（省略若干父进程）。</font>

<font style="color:#3b3b3b;">本节通过系统调用</font><font style="color:#3b3b3b;">waitpid</font><font style="color:#3b3b3b;">在父进程中等待子进程完成并执行回收工作。</font>

#### <font style="color:#3b3b3b;">函数原型</font>

<font style="color:#3b3b3b;">执行man 2 waitpid查看手册可知声明如下：</font>

```cpp
#include <sys/types.h>   
#include <sys/wait.h>   

/** 等待子进程的终止并获取子进程的退出状态
*    功能简单 没有选择
*/
pid_t wait(int *wstatus);

/**
 * 功能灵活 可以设置不同的模式 可以等待特定的子进程
 * * pid: 等待的模式
 * (1) < -1 : 等待进程组ID等于 |pid| (pid的绝对值) 的所有子进程
 * (2) = -1 : 等待任何子进程（等同于 wait 函数） -> ⚠️ 仅限直系子进程，不含孙进程
 * (3) = 0  : 等待与调用者处于同一进程组的任何子进程
 * (4) > 0  : 等待进程ID等于 pid 的特定子进程
 *
 * wstatus: 输出型参数。
 * ⚠️ 注意：不能直接读取，需配合宏使用（如 WEXITSTATUS, WIFSIGNALED 等）来解析退出码或终止信号。
 *
 * options: 选项 (可按位或 | )
 * (1) WNOHANG    : 非阻塞模式。如果子进程未结束，立即返回0，不傻等。
 * (2) WUNTRACED  : 如果子进程暂停（Stop，如收到 SIGSTOP），也返回状态。
 * (3) WCONTINUED : 如果暂停的子进程被恢复（收到 SIGCONT），也返回状态。
 *
 * return:
 * > 0 : 成功，返回结束（或状态改变）的子进程 PID
 * = 0 : 使用了 WNOHANG 且子进程尚未结束
 * = -1: 出错（如没有子进程了，或被信号中断），具体看 errno
 */
pid_t waitpid(pid_t pid, int *wstatus, int options);

/*
    更加全面的子进程监控和状态报告
*/
int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
```

#### <font style="color:#3b3b3b;">测试例程</font>

<font style="color:#3b3b3b;">创建waitpid_test.c，写入以下内容。</font>

```cpp
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char const* argv[]) {
  // fork之前
  int subprocess_status;
  printf("老学员在校区\n");
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return 1;
  } else if (pid == 0) {
    // 新学员
    char* args[] = {"/usr/bin/ping", "-c", "50", "www.atguigu.com", NULL};
    char* envs[] = {NULL};
    printf("新学员%d联系海哥10次\n", getpid());
    int exR = execve(args[0], args, envs);
    if (exR < 0) {
      perror("execve");
      return 1;
    }

  } else {
    // 老学员
    printf("老学员%d等待新学员%d联系\n", getpid(), pid);
    waitpid(pid, &subprocess_status, 0);
  }

  printf("老学员等待新学员联系完成\n");
  return 0;
}

```

<font style="color:#3b3b3b;">waitpid(pid, </font><font style="color:#3b3b3b;">&subprocess_status,</font><font style="color:#3b3b3b;"> 0)</font><font style="color:#3b3b3b;">执行之后将会一直挂起父进程（调用者）的执行，直至子进程</font><font style="color:#3b3b3b;">pid</font><font style="color:#3b3b3b;">终止。</font>

#### <font style="color:#3b3b3b;">Makefile</font>

```makefile
waitpid_test: waitpid_test.c
 -$(CC) -o $@ $^
 -./$@
 -rm ./$@
```

#### 运行

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763520779937-ff45ad98-4231-4be6-ba81-cee358f29d77.png)

（1）在运行的过程中使用ps -ef | grep ping查看进程

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763520793705-b701e219-d7d9-4741-b38e-decfcbc12edc.png)

<font style="color:#3b3b3b;">此时父进程没有结束，而是在等待子进程</font>

（2）<font style="color:#3b3b3b;">子进程执行完毕后，重新查看进程</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763520808042-00f3cb4b-14ba-4f27-9f38-8002a0264767.png)

<font style="color:#3b3b3b;">此时父进程和子进程全部销毁，父进程在子进程结束后回收了它，然后运行结束，由其父进程回收。</font>

### 进程树
>
> [视频讲解](https://bilibili.com/video/BV1DJ4m1M77z?vd_source=e7b9c9d6f1723575486b50acd6e7929f&spm_id_from=333.788.videopod.episodes&p=39)
>

<font style="color:#3b3b3b;">Linux</font><font style="color:#3b3b3b;">的进程是通过父子关系组织起来的，所有进程之间的父子关系共同构成了进程树（</font><font style="color:#3b3b3b;">Process Tree</font><font style="color:#3b3b3b;">）。进程树中每个节点都是其上级节点的子进程，同时又是子结点的父进程。一个进程的父进程只能有一个，而一个进程的子进程可以不止一个。</font>

#### 创建pstree_test.c，写入以下内容

<font style="color:#3b3b3b;">以下程序是在fork_execve_test.c的基础上做了改动，通过fgetc()阻塞父进程的执行，确保测试期间父进程不会退出。</font>

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const* argv[]) {
    char* name = "老学员";
    printf("%s%d在一楼精进\n", name, getpid());
    __pid_t pid = fork();
    if (pid == -1) {
        printf("邀请新学员失败!\n");
    } else if (pid == 0) {
        // 新学员在这里
        char* newName = "ergou";
        char* argv[] = {"/home/atguigu/process_test/erlou", newName, NULL};
        char* envp[] = {
        "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/"
        "usr/games:/usr/local/games:/snap/bin",
        NULL};
        int re = execve(argv[0], argv, envp);
        if (re == -1) {
            printf("新学员上二楼失败\n");
            return 1;
        }

    } else {
        // 老学员在这里
        printf("老学员%d邀请完%d之后还是在一楼学习\n", getpid(), pid);
        // 等待新学员二楼结束 手动输入一个字母结束等待
        char bye = fgetc(stdin);
    }
    return 0;
}

```

#### Makefile

```makefile
pstree_test: pstree_test.c erlou
    -$(CC) -o $@ $<
    -./$@
    -rm ./$@ ./$(lastword $^)
```

#### 运行测试

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521482903-3575bf1e-8956-43d0-8bb7-52b7cb5115d1.png)

（1）此时在终端命令行执行ps -ef查看进程

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521495870-a383a54c-c13a-4518-9f91-f3c6da723a51.png)

<font style="color:#3b3b3b;">此时通过</font><font style="color:#3b3b3b;">fork()</font><font style="color:#3b3b3b;">和</font><font style="color:#3b3b3b;">execve()</font><font style="color:#3b3b3b;">启动的子进程共存，我们可以追溯</font><font style="color:#3b3b3b;">main</font><font style="color:#3b3b3b;">进程的父级进程，上推可以看到其三级父进程的</font><font style="color:#3b3b3b;">pid</font><font style="color:#3b3b3b;">为</font><font style="color:#3b3b3b;">11614</font><font style="color:#3b3b3b;">。</font>

（2）<font style="color:#3b3b3b;">执行命令ps -ef | grep 11614</font>

（3）<font style="color:#3b3b3b;">执行命令ps -ef | grep 11612</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528314-ff937a7e-ff91-4b8d-9717-b69fe0575af9.png)

（4）<font style="color:#3b3b3b;">执行命令ps -ef | grep 10295</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528651-8402b153-4244-4e86-abdb-7a46fb9bdba5.png)

（5）<font style="color:#3b3b3b;">执行命令ps -ef | grep 3444</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528666-e9947236-dcd0-4ae3-b1da-94aa03acff10.png)

（6）<font style="color:#3b3b3b;">执行命令ps -ef | grep 3316</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528670-84dbe791-0cea-4ecd-a62b-364bbaf8fc5e.png)

（7）<font style="color:#3b3b3b;">执行命令ps -ef | grep 1582</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528647-2a3ed5cc-a86a-44d9-99fe-3c6c59e98e7f.png)

<font style="color:#3b3b3b;">可以看到</font><font style="color:#3b3b3b;">pid</font><font style="color:#3b3b3b;">为</font><font style="color:#3b3b3b;">1582</font><font style="color:#3b3b3b;">的进程，它的父进程为</font><font style="color:#3b3b3b;">1</font><font style="color:#3b3b3b;">号进程，这个进程是操作系统的第一个进程，是所有其它进程的父进程</font>

（8）<font style="color:#3b3b3b;">执行命令ps -ef</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528873-74842550-7b3e-4a4d-af0a-22645c2bb1d5.png)

<font style="color:#3b3b3b;">可以看到</font><font style="color:#3b3b3b;">pid</font><font style="color:#3b3b3b;">为</font><font style="color:#3b3b3b;">1</font><font style="color:#3b3b3b;">的进程是由</font><font style="color:#3b3b3b;">/sbin/init splash</font><font style="color:#3b3b3b;">命令创建的。</font>

（9）<font style="color:#3b3b3b;">执行ll /sbin/init</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528936-ad1efe4b-a5a0-4831-820b-1ae85f463698.png)

<font style="color:#3b3b3b;">实质上，</font><font style="color:#3b3b3b;">1</font><font style="color:#3b3b3b;">号进程就是</font><font style="color:#3b3b3b;">systemd</font><font style="color:#3b3b3b;">，它由内核创建，是第一个进程，负责初始化系统，启动其他所有用户空间的服务和进程。它是所有进程的祖先。</font>

<font style="color:#3b3b3b;">在</font><font style="color:#3b3b3b;">ps -ef</font><font style="color:#3b3b3b;">的输出结果中，我们发现，</font><font style="color:#3b3b3b;">CMD</font><font style="color:#3b3b3b;">部分有的行带有</font><font style="color:#3b3b3b;">[]</font><font style="color:#3b3b3b;">，而有的没有，前者属于内核线程，内核线程在内核空间执行，不占用任何用户空间资源，它们在技术上是线程，而在许多方面表现得像独立的进程，因此也会被</font><font style="color:#3b3b3b;">ps</font><font style="color:#3b3b3b;">命令检索到。第一个内核线程的</font><font style="color:#3b3b3b;">pid</font><font style="color:#3b3b3b;">为</font><font style="color:#3b3b3b;">2</font><font style="color:#3b3b3b;">，它是所有其它内核线程的祖先。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528996-95ed63fc-bc13-4848-b110-ed175781d7ce.png)

<font style="color:#3b3b3b;">此外，</font><font style="color:#3b3b3b;">pid</font><font style="color:#3b3b3b;">为</font><font style="color:#3b3b3b;">1</font><font style="color:#3b3b3b;">的进程和</font><font style="color:#3b3b3b;">pid</font><font style="color:#3b3b3b;">为</font><font style="color:#3b3b3b;">2</font><font style="color:#3b3b3b;">的内核线程，它们的父进程</font><font style="color:#3b3b3b;">id</font><font style="color:#3b3b3b;">均为</font><font style="color:#3b3b3b;">0</font><font style="color:#3b3b3b;">，这是因为二者都是由内核直接创建，都是“祖先”，不存在“辈分”更大的进程或内核线程。</font>

（10）<font style="color:#3b3b3b;">pstree查看进程树</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528975-5f24a8d7-cae9-4d48-9c2d-3eac42da1828.png)

<font style="color:#3b3b3b;">pstree</font><font style="color:#3b3b3b;">命令会以树状图展示所有用户进程的依赖关系，可以看到，</font><font style="color:#3b3b3b;">systemd</font><font style="color:#3b3b3b;">是所有用户进程的祖先。</font>

（11）<font style="color:#3b3b3b;">pstree -p查看进程树</font>

<font style="color:#3b3b3b;">-p</font><font style="color:#3b3b3b;">表示显示进程号</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521528989-9984c18c-5acf-46d8-9427-b515e6af52f6.png)

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521529184-5e1b00d9-f53a-4995-b369-3cc91944492f.png)

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521529270-b0c42e8a-eda4-4aed-b6e5-c9c428f805aa.png)

<font style="color:#3b3b3b;">可以看到，ping进程的父子关系正如我们通过ps -ef | grep [pid]的方式逐级检索查明的那样。</font>

### 孤儿进程
>
> [视频讲解](https://bilibili.com/video/BV1DJ4m1M77z?vd_source=e7b9c9d6f1723575486b50acd6e7929f&spm_id_from=333.788.videopod.episodes&p=40)
>

<font style="color:#3b3b3b;">孤儿进程（</font><font style="color:#3b3b3b;">Orphan Process</font><font style="color:#3b3b3b;">）是指父进程已结束或终止，而它仍在运行的进程。</font>

<font style="color:#3b3b3b;">当父进程结束之前没有等待子进程结束，且父进程先于子进程结束时，那么子进程就会变成孤儿进程。</font>

#### <font style="color:#3b3b3b;">创建erlou_block.c，写入以下内容</font>

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("参数不够,上不了二楼.\n");
        return 1;  // 当没有传入参数时，应返回非零值表示错误
    }
    printf("我是%s %d,我跟海哥上二楼啦!\n", argv[1], getpid());
    // 挂起子进程
    sleep(100);
    return 0;
}

```

#### 创建orphan_process_test.c，写入以下内容

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const* argv[]) {
  char* name = "老学员";
  printf("%s%d在一楼精进\n", name, getpid());
  __pid_t pid = fork();
  if (pid == -1) {
    printf("邀请新学员失败!\n");
  } else if (pid == 0) {
    // 新学员在这里
    char* newName = "ergou";
    char* argv[] = {"/home/atguigu/process_test/erlou_block", newName, NULL};
    char* envp[] = {
        "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/"
        "usr/games:/usr/local/games:/snap/bin",
        NULL};
    int re = execve(argv[0], argv, envp);
    if (re == -1) {
      printf("新学员上二楼失败\n");
      return 1;
    }
  } else {
    // 老学员在这里
    printf("老学员%d邀请完%d之后还是在一楼学习\n", getpid(), pid);
  }
  return 0;
}

```

#### Makefile

```makefile
erlou_block: erlou_block.c
    -$(CC) -o $@ $^

orphan_process_test: orphan_process_test.c erlou_block
    -$(CC) -o $@ $<
    -./$@
    -rm ./$@ ./$(word 2, $^)
```

#### 重新执行程序

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521741432-21f3ebb6-7952-4d44-8301-18ae3c17f441.png)

（1）<font style="color:#3b3b3b;">在终端命令行通过</font><font style="color:#3b3b3b;">ps -ef</font><font style="color:#3b3b3b;">  </font><font style="color:#3b3b3b;">| grep 12463</font><font style="color:#3b3b3b;">查看进程</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521754315-7090f509-d8ab-44b3-893d-818a0dd99ae7.png)

<font style="color:#3b3b3b;">可以看到子进程已被其祖先领养，此时的进程树如下（进程树过长，省略无关紧要的部分）</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521754289-287b3dba-bb4c-477d-92a2-1f0f74a7371a.png)

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521754539-84fc1c55-9f7a-4b5a-9a99-da5ba3f7409d.png)

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763521754554-9bfad388-17c7-4ecb-a776-2b4209a48215.png)

<font style="color:#3b3b3b;">我们可以得出结论：孤儿进程会被其祖先自动领养。此时的子进程因为和终端切断了联系，所以很难再进行标准输入使其停止了，所以写代码的时候一定要注意避免出现孤儿进程。</font>

## 进程间通信

<font style="color:#3b3b3b;">进程之前的内存是隔离的，如果多个进程之间需要进行信息交换，常用的方法有以下几种：</font>

（1）<font style="color:#3b3b3b;">Unix Domain Socket IPC</font>

（2）<font style="color:#3b3b3b;">管道（有名管道、无名管道）</font>

（3）<font style="color:#3b3b3b;">共享内存</font>

（4）<font style="color:#3b3b3b;">消息队列</font>

（5）<font style="color:#3b3b3b;">信号量</font>

<font style="color:#3b3b3b;">此处提到的Unix Domain Socket IPC和信号量放在后面的章节讲解。</font>

### SystemV IPC和POSIX IPC

#### System V

<font style="color:#3b3b3b;">System V（读作System Five）是一种基于UNIX的操作系统版本，最初由AT&T（American TelePhone and Telegraph Company，美国电话电报公司，由Bell TelePhone Company发展而来）开发。它在1983年首次发布，对UNIX操作系统的发展产生了深远的影响。SystemV引入了许多新的特性和标准，后来被许多UNIX系统和类UNIX系统（如Linux）采纳。</font>

#### System V IPC

<font style="color:#3b3b3b;">System V IPC</font><font style="color:#3b3b3b;">（</font><font style="color:#3b3b3b;">Inter-Process Communication</font><font style="color:#3b3b3b;">，进程间通信）是</font><font style="color:#3b3b3b;">System V</font><font style="color:#3b3b3b;">操作系统引入的一组进程间通信机制，包括</font>**<font style="color:red;">消息队列</font>**<font style="color:#3b3b3b;">、</font>**<font style="color:red;">信号量</font>**<font style="color:#3b3b3b;">和</font>**<font style="color:red;">共享内存</font>**<font style="color:#3b3b3b;">。这些机制允许不同的进程以一种安全且高效的方式共享数据和同步操作。</font>

（1）<font style="color:#3b3b3b;">消息队列：允许进程以消息的形式交换数据，这些消息存储在队列中，直到它们被接收。</font>

（2）<font style="color:#3b3b3b;">信号量：主要用于进程间的同步，防止多个进程同时访问相同的资源。</font>

（3）<font style="color:#3b3b3b;">共享内存：允许多个进程访问同一块内存区域，提供了一种非常高效的数据共享方式。</font>

<font style="color:#3b3b3b;">System V IPC是UNIX和类UNIX系统中常用的IPC方法之一，它</font>**<font style="color:red;">通过关键字</font>**<font style="color:#3b3b3b;">（key）来标识和访问IPC资源。</font>

#### POSIX IPC

<font style="color:#3b3b3b;">POSIX IPC</font><font style="color:#3b3b3b;">是</font><font style="color:#3b3b3b;">POSIX</font><font style="color:#3b3b3b;">标准中的一部分，提供了一种更现代和标准化的进程间通信方式，同样包括消息队列、信号量和共享内存三种方式。</font>

（1）<font style="color:#3b3b3b;">消息队列：类似于</font><font style="color:#3b3b3b;">System V</font><font style="color:#3b3b3b;">，但通常具有更简洁的</font><font style="color:#3b3b3b;">API</font><font style="color:#3b3b3b;">和更好的错误处理能力。</font>

（2）<font style="color:#3b3b3b;">信号量：提供了更多的功能和更高的性能，支持更大范围的操作。</font>

（3）<font style="color:#3b3b3b;">共享内存：提供了更多的控制和配置选项，以支持更复杂的应用场景。</font>

<font style="color:#3b3b3b;">POSIX IPC 使用名字（name）作为唯一标识。这些名字通常是以正斜杠（/）开头的字符串，用于唯一地识别资源如消息队列、信号量或共享内存对象。</font>

#### 二者的比较

<font style="color:#3b3b3b;">System V IPC</font><font style="color:#3b3b3b;">和</font><font style="color:#3b3b3b;">POSIX IPC</font><font style="color:#3b3b3b;">在功能上有所重叠，但它们在实现和</font><font style="color:#3b3b3b;">API</font><font style="color:#3b3b3b;">设计上有明显的区别。</font><font style="color:#3b3b3b;">POSIX IPC</font><font style="color:#3b3b3b;">通常被视为更现代、更标准化的解决方案，提供了更好的跨平台支持和更易于使用的</font><font style="color:#3b3b3b;">API</font><font style="color:#3b3b3b;">。然而，</font><font style="color:#3b3b3b;">System V IPC</font><font style="color:#3b3b3b;">在历史上更早地被大量</font><font style="color:#3b3b3b;">UNIX</font><font style="color:#3b3b3b;">系统所采用，因此在一些旧的或特定的环境中仍然非常重要。在选择使用哪种</font><font style="color:#3b3b3b;">IPC</font><font style="color:#3b3b3b;">机制时，应考虑应用程序的具体需求、目标系统的支持程度以及开发者的熟悉程度。</font>

<font style="color:#3b3b3b;">System V IPC和POSIX IPC各自提供了一组API，如果全部介绍未免太过冗长，他们实现的效果是类似的。本文只介绍POSIX IPC提供的API。</font>

### 匿名管道（Pipe）
>
> [管道资料](https://www.yuque.com/tianchungu/operating_system/nq51cetu0xftgm7e)
>

#### 库函数perror()

（1）创建perror_test.c，写入以下内容。

```cpp
#include <stdio.h>

int main(int argc, char const *argv[])
{
    /**
     * 输出错误信息报告到系统的错误输出
     * char *__s: 自定义的错误信息前缀 会打印在输出的前面 中间补充": " 后面跟errno
     * 隐藏参数: errno 用于保存错误信息的全局变量 系统调用和库函数出错会将信息存储到这里
     * void perror (const char *__s)
    */
    fopen("bucunzai.txt","r");
    perror("这道题我不会做! ");
    return 0;
}
```

（2）Makefile

```makefile
perror_test: perror_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）运行

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763523008584-a259c587-8545-4d13-9721-611781f841cb.png)

（4）<font style="color:#3b3b3b;">分析</font>

<font style="color:#3b3b3b;">当系统调用或库函数发生错误时，通常会通过设置全局变量</font><font style="color:#3b3b3b;">errno</font><font style="color:#3b3b3b;">来指示错误的具体原因。</font><font style="color:#3b3b3b;">errno</font><font style="color:#3b3b3b;">是在</font><font style="color:#3b3b3b;">C</font><font style="color:#3b3b3b;">语言（及其在</font><font style="color:#3b3b3b;">Unix</font><font style="color:#3b3b3b;">、</font><font style="color:#3b3b3b;">Linux</font><font style="color:#3b3b3b;">系统下的应用）中用来存储错误号的一个全局变量。每当系统调用或某些库函数遇到错误，无法正常完成操作时，它会将一个错误代码存储到</font><font style="color:#3b3b3b;">errno</font><font style="color:#3b3b3b;">中。这个错误代码提供了失败的具体原因，程序可以通过检查</font><font style="color:#3b3b3b;">errno</font><font style="color:#3b3b3b;">的值来确定发生了什么错误，并据此进行相应的错误处理。</font>

<font style="color:#3b3b3b;">errno</font><font style="color:#3b3b3b;">定义在头文件</font><font style="color:#3b3b3b;"><errno.h></font><font style="color:#3b3b3b;">中，引入该文件即可调用全局变量</font><font style="color:#3b3b3b;">errno</font><font style="color:#3b3b3b;">。</font>

<font style="color:#3b3b3b;">perror</font><font style="color:#3b3b3b;">函数用于将</font><font style="color:#3b3b3b;">errno</font><font style="color:#3b3b3b;">当前值对应的错误描述以人类可读的形式输出到标准错误输出（</font><font style="color:#3b3b3b;">stderr</font><font style="color:#3b3b3b;">）。</font>

<font style="color:#3b3b3b;">参数</font><font style="color:#3b3b3b;">s</font><font style="color:#3b3b3b;">：指向一个字符串的指针，如果</font><font style="color:#3b3b3b;">s</font><font style="color:#3b3b3b;">不是空指针且指向的不是</font><font style="color:#3b3b3b;">\0</font><font style="color:#3b3b3b;">字符，则</font><font style="color:#3b3b3b;">perror</font><font style="color:#3b3b3b;">会在</font><font style="color:#3b3b3b;">s</font><font style="color:#3b3b3b;">后添加一个冒号和空格作为前缀，输出错误信息，否则不输出前缀，直接输出错误信息。</font>

（5）<font style="color:#3b3b3b;">创建errno_test.c</font>

```cpp
#include <errno.h>
#include <stdio.h>

int main() {

    fopen("/opt", "a+");

    printf("errno: %d\n", errno);
    perror("文件打开出问题");

    return 0;
}
```

（6）Makefile

```makefile
errno_test: errno_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（7）运行结果如下

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763523118581-58291a90-eef6-4400-8bd4-77b5f7ccd9a3.png)

#### 系统调用pipe()

<font style="color:#3b3b3b;">匿名管道是位于内核的一块缓冲区，用于进程间通信。创建匿名管道的系统调用为pipe。执行man 2 pipe查看手册，其声明如下：</font>

```cpp
#include <unistd.h>

/**
 * 在内核空间创建管道，用于父子进程或者其他相关联的进程之间通过管道进行双向的数据传输。。
 * 
 * pipefd: 用于返回指向管道两端的两个文件描述符。pipefd[0]指向管道的读端。pipefd[1]指向管道的写端。
 * return: 成功   0
 *         不成功 -1，并且pipefd不会改变
 */
int pipe(int pipefd[2]);
```

#### 匿名管道测试例程

（1）宏定义EXIT_FAILURE、EXIT_SUCCESS 、STDOUT_FILENO

```cpp
#define EXIT_FAILURE    1   /* Failing exit status.  */
#define EXIT_SUCCESS    0   /* Successful exit status.  */

#define STDIN_FILENO    0   /* Standard input.  */
#define STDOUT_FILENO   1   /* Standard output.  */
#define STDERR_FILENO   2   /* Standard error output.  */
```

+ EXIT_FAILURE：表示以失败状态退出进程
+ EXIT_SUCCESS：表示以成功状态退出进程
+ STDOUT_FILENO：表示标准输出

（2）创建unnamed_pipe_test.c

<font style="color:#3b3b3b;">下面的例子展示了父进程将argv[1]写入匿名管道，子进程读取并输出到控制台的过程，例程如下。</font>

```cpp
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    
    int pipefd[2];
    pid_t cpid;
    char buf;
    if (argc != 2)
    {
        fprintf(stderr, "%s:请填写需要传递的信息\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (pipe(pipefd) == -1)
    {
        perror("创建管道失败\n");
        exit(EXIT_FAILURE);
    }
    cpid = fork();
    if (cpid == -1)
    {
        perror("邀请新学员失败!\n");
        exit(EXIT_FAILURE);
    }
    if (cpid == 0)
    {       
        // 新学员读数据 关闭写端              
        close(pipefd[1]); 
        char str[100]={0};
        sprintf(str,"新学员%d收到邀请\n",getpid());
        write(STDOUT_FILENO, str, sizeof(str));
        // 一直读取读端的数据 单个字节读取方便读取结尾 直到数据结束或出错
        while (read(pipefd[0], &buf, 1) > 0){
            // 将读取数据写到标准输出
            write(STDOUT_FILENO, &buf, 1);
        }
        // 输出换行
        write(STDOUT_FILENO, "\n", 1);
        close(pipefd[0]);
        _exit(EXIT_SUCCESS);
    }
    else
    { 
        // 老学员写数据 关闭读端
        close(pipefd[0]); 
        // 写入传入的参数到管道的写端
        printf("老学员%d发出邀请\n",getpid());
        write(pipefd[1], argv[1], strlen(argv[1]));
        // 写完之后关闭写端  读端会返回0
        close(pipefd[1]); 
        // 等待子进程结束
        waitpid(cpid,NULL,0);       
        exit(EXIT_SUCCESS);
    }
}

```

（3）Makefile

```makefile
unnamed_pipe_test: unnamed_pipe_test.c
    -$(CC) -o $@ $<
    -./$@ "test"
    -rm ./$@
```

<font style="color:#3b3b3b;">需要注意的是，本例中我们需要通过命令行传参程序才能正确执行，</font><font style="color:#3b3b3b;">-./$@</font><font style="color:#3b3b3b;">之后的</font><font style="color:#3b3b3b;">"test"</font><font style="color:#3b3b3b;">就是传递给程序的命令行参数。</font>

（4）<font style="color:#3b3b3b;">运行</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763523422706-5f373d43-3aa2-4246-9dca-8fe1ee007119.png)

<font style="color:#3b3b3b;">可以看到命令行传入的参数被打印到了终端，我们在上述程序中通过父进程将数据写入了管道，然后子进程从管道读出数据，写入了标准输出，日志如上则测试通过。</font>

#### 使用管道的限制

（1）<font style="color:#3b3b3b;">两个进程通过一个管道只能实现单向通信，比如上面的例子，父进程写子进程读，如果有时候也需要子进程写父进程读，就必须另开一个管道。</font>

（2）<font style="color:#3b3b3b;">管道的读写端通过打开的文件描述符来传递，因此要通信的两个进程必须从它们的公共祖先那里继承管道文件描述符。上面的例子是父进程把文件描述符传给子进程之后父子进程之间通信，也可以父进程</font><font style="color:#3b3b3b;">fork</font><font style="color:#3b3b3b;">两次，把文件描述符传给两个子进程，然后两个子进程之间通信，总之需要通过</font><font style="color:#3b3b3b;">fork</font><font style="color:#3b3b3b;">传递文件描述符使两个进程都能访问同一管道，它们才能通信。</font>

#### 管道返回的文件描述符

<font style="color:#3b3b3b;">管道返回的两个文件描述符分别表示读写，各自指向一个</font><font style="color:#3b3b3b;">struct file</font><font style="color:#3b3b3b;">结构体，然而，它们并不对应真正的文件。</font>

<font style="color:#3b3b3b;">struct file的私有数据指针属性private_data指向struct pipe_inode_info类型的结构体，后者声明位于/usr/src/linux-hwe-6.5-headers-6.5.0-27/include/linux/pipe_fs_i.h，定义如下。</font>

```cpp
struct pipe_inode_info {
    struct mutex mutex;                 // 互斥锁，用于同步对管道的访问
    wait_queue_head_t rd_wait, wr_wait; // 等待队列，分别用于读操作和写操作的阻塞管理
    unsigned int head;                  // 管道缓冲区中读取数据的起始位置
    unsigned int tail;                  // 管道缓冲区中写入数据的结束位置
    unsigned int max_usage;             // 管道使用过的最大容量，用于监控和调优
    unsigned int ring_size;             // 管道缓冲区的总大小（环形缓冲区的大小）
    unsigned int nr_accounted;          // 已记录的管道使用量
    unsigned int readers;               // 当前打开管道读端的文件描述符数量
    unsigned int writers;               // 当前打开管道写端的文件描述符数量
    unsigned int files;                 // 总共打开的文件描述符数量
    unsigned int r_counter;             // 读操作的计数器
    unsigned int w_counter;             // 写操作的计数器
    bool poll_usage;                    // 标志位，指示是否有进程在使用poll等机制监视管道
#ifdef CONFIG_WATCH_QUEUE
    bool note_loss; // 是否记录了通知的丢失（用于调试或特定的资源监控）
#endif
    struct page *tmp_page;                // 用于管道操作的临时内存页，通常用于优化读写性能
    struct fasync_struct *fasync_readers; // 异步读操作通知结构体，支持SIGIO信号
    struct fasync_struct *fasync_writers; // 异步写操作通知结构体，支持SIGIO信号
    struct pipe_buffer *bufs;             // 指向管道缓冲区数组的指针，每个元素管理一个缓冲块
    struct user_struct *user;             // 指向代表管道拥有者的用户结构体
#ifdef CONFIG_WATCH_QUEUE
    struct watch_queue *watch_queue; // 用于管理内核通知队列的结构体（用于高级事件监视功能）
#endif
};

```

<font style="color:#3b3b3b;">当我们开启管道时，父进程会创建两个struct file结构体用于管道的读写操作，并为二者各自分配一个文件描述符。它们的private_data属性指向同一个struct pipe_inode_info的结构体，由后者管理对于管道缓冲区的读写。通过fork()创建一个子进程，后者会继承文件描述符，指向相同的struct file结构体，如下。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763523478780-21043dbd-4611-4097-b822-6487e3148b8a.png)

#### 图解

<font style="color:#3b3b3b;">进程间通过管道通信的过程如下。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763523502541-b7c2fb55-14df-45e1-bf96-c690fce913e2.png)

### 有名管道（FIFO）

<font style="color:#3b3b3b;">上文介绍的</font><font style="color:#3b3b3b;">Pipe</font><font style="color:#3b3b3b;">是匿名管道，只能在有父子关系的进程间使用，某些场景下并不能满足需求。与匿名管道相对的是有名管道，在</font><font style="color:#3b3b3b;">Linux</font><font style="color:#3b3b3b;">中称为</font><font style="color:#3b3b3b;">FIFO</font><font style="color:#3b3b3b;">，即</font><font style="color:#3b3b3b;">First In First Out</font><font style="color:#3b3b3b;">，先进先出队列。</font>

<font style="color:#3b3b3b;">FIFO</font><font style="color:#3b3b3b;">和</font><font style="color:#3b3b3b;">Pipe</font><font style="color:#3b3b3b;">一样，提供了双向进程间通信渠道。但要注意的是，无论是有名管道还是匿名管道，</font>**<font style="color:red;">同一条管道只应用于单向通信</font>**<font style="color:#3b3b3b;">，否则可能出现通信混乱（进程读到自己发的数据）。</font>

<font style="color:#3b3b3b;">有名管道可以用于任何进程之间的通信。</font>

#### 库函数mkfifo()

<font style="color:#3b3b3b;">执行man 3 mkfifo查看文件说明。</font>

```cpp
#include <sys/types.h>
#include <sys/stat.h>

/**
 * @brief 用于创建有名管道。该函数可以创建一个路径为pathname的FIFO专用文件，mode指定了FIFO的权限，FIFO的权限和它绑定的文件是一致的。FIFO和pipe唯一的区别在于创建方式的差异。一旦创建了FIFO专用文件，任何进程都可以像操作文件一样打开FIFO，执行读写操作。
 * 
 * @param pathname 有名管道绑定的文件路径
 * @param mode 有名管道绑定文件的权限
 * @return int 
 */
int mkfifo(const char *pathname, mode_t mode);
```

#### 有名管道发送端

<font style="color:#3b3b3b;">创建fifo_write.c，写入以下内容。</font>

```cpp
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  int fd;
  char* pipe_path = "/tmp/myfifo";

  // 创建有名管道，权限设置为 0664
  if (mkfifo(pipe_path, 0664) != 0) {
    perror("mkfifo failed");
    if (errno != 17) {
      exit(EXIT_FAILURE);
    }
  }

  // 打开有名管道用于写入
  fd = open(pipe_path, O_WRONLY);
  if (fd == -1) {
    perror("open failed");
    exit(EXIT_FAILURE);
  }

  char write_buf[100];
  ssize_t read_num;

  while ((read_num = read(STDIN_FILENO, write_buf, 100)) > 0) {
    write(fd, write_buf, read_num);
  }

  if (read_num < 0) {
    perror("read");
    printf("命令行数据读取异常，退出");
    close(fd);
    exit(EXIT_FAILURE);
  }

  printf("发送管道退出，进程终止\n");
  close(fd);

  return 0;
}

```

#### 有名管道接收端

<font style="color:#3b3b3b;">创建fifo_read.c，写入以下内容。</font>

```cpp
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

```

#### Makefile

（1）<font style="color:#3b3b3b;">声明伪目标</font>

<font style="color:#3b3b3b;">在文件开头补充</font>

```makefile
.PHONY: named_fifo named_fifo_cleanm
```

<font style="color:#3b3b3b;">如下</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527592379-4e1cfa72-a41a-4843-b78a-d799c369f05e.png)

（2）补充编译和清除target

```makefile
fifo_write: fifo_write.c
    $(CC) -o $@ $<

fifo_read: fifo_read.c
    $(CC) -o $@ $<

named_fifo: fifo_write fifo_read

named_fifo_clean:
    -rm ./fifo_write ./fifo_read
```

#### 测试

（1）编译

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527638099-3fae8e0b-a1cb-449a-90f7-b1cfe1882920.png)

（2）在XShell开启两个标签页，右键点击“垂直分割”

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527653248-84181188-d4a7-45f5-b718-af253b2e7877.png)

（3）在两个标签页分别开启发送端和接收端

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527671650-99ec69c4-921f-4880-8b58-2f811b884c46.png)

（4）在发送端发送数据

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527686048-c029f739-68c1-4ba8-97a1-ac301440ca30.png)

<font style="color:#3b3b3b;">接收端可以看到数据。</font>

（5）<font style="color:#3b3b3b;">终止进程</font>

<font style="color:#3b3b3b;">发送端按下Ctrl+D，终止发送端，接收端收到信号，也会退出。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527701511-780cd488-9c29-4ba8-8a83-512cf998e0aa.png)

#### 有名管道在文件系统的对应

（1）再次启动

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527721156-364f0667-6689-45bd-8f06-12ed3ff17664.png)

<font style="color:#3b3b3b;">再次启动上述文件提示</font><font style="color:#3b3b3b;">mkfifo</font><font style="color:#3b3b3b;">失败，原因是文件已存在，那么这里的文件是指哪个文件？</font>

（2）<font style="color:#3b3b3b;">fifo专用文件</font>

<font style="color:#3b3b3b;">创建有名管道时，我们同时创建了一个fifo专用文件，在上述案例中是/tmp/myfifo，这实际上就是fifo专用文件在文件系统的路径，如下。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527743699-b555e1d8-982f-41c3-8169-13714a196032.png)

<font style="color:#3b3b3b;">程序执行完毕之后，该文件未被清除，因此会出现上述报错。</font>

（3）<font style="color:#3b3b3b;">更改fifo_write.c</font>

<font style="color:#3b3b3b;">有名管道使用完成后，应该通过unlink调用清除相关资源。这个函数只能调用一次，重复清除会提示No such file or directory。</font>

```cpp
#include <unistd.h>

/**
 * @brief 从文件系统中清除一个名称及其链接的文件
 * 
 * @param pathname 文件路径
 * @return int 成功返回0，失败返回-1，并设置errno
 */
int unlink(const char *pathname);
```

<font style="color:#3b3b3b;">我们在fifo_write.c中补充资源释放操作，如下。</font>

```cpp
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main()
{
    int fd;
    char *pipe_path = "/tmp/myfifo";

    // 创建有名管道，权限设置为 0664
    if (mkfifo(pipe_path, 0664) != 0)
    {
        perror("mkfifo failed");
        if (errno != 17)
        {
            exit(EXIT_FAILURE);
        }
    }

    // 打开有名管道用于写入
    fd = open(pipe_path, O_WRONLY);
    if (fd == -1)
    {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    char write_buf[100];
    ssize_t read_num;

    while ((read_num = read(STDIN_FILENO, write_buf, 100)) > 0) {
        write(fd, write_buf, read_num);
    }

    if (read_num < 0) {
        perror("read");
        printf("命令行数据读取异常，退出");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    printf("发送管道退出，进程终止\n");
    close(fd);

    if(unlink(pipe_path) == -1) {
        perror("fifo_write unlink");
    }

    return 0;
}

```

<font style="color:#3b3b3b;">接下来，首先清除之前遗留的</font><font style="color:#3b3b3b;">/tmp/myfifo</font><font style="color:#3b3b3b;">，清除</font><font style="color:#3b3b3b;">fifo_read</font><font style="color:#3b3b3b;">和</font><font style="color:#3b3b3b;">fifo_write</font><font style="color:#3b3b3b;">。然后重新编译运行，再停止读写进程，可以发现</font><font style="color:#3b3b3b;">fifo</font><font style="color:#3b3b3b;">专用文件已被清除。</font>

#### 注意

<font style="color:#3b3b3b;">调用</font><font style="color:#3b3b3b;">open()</font><font style="color:#3b3b3b;">打开有名管道时，</font><font style="color:#3b3b3b;">flags</font><font style="color:#3b3b3b;">设置为</font><font style="color:#3b3b3b;">O_WRONLY</font><font style="color:#3b3b3b;">则当前进程用于向有名管道写入数据，设置为</font><font style="color:#3b3b3b;">O_RDONLY</font><font style="color:#3b3b3b;">则当前进程用于从有名管道读取数据。设置为</font><font style="color:#3b3b3b;">O_RDWR</font><font style="color:#3b3b3b;">从技术上是可行的，但正如上文提到的，此时管道既读又写很可能导致一个进程读取到自己发送的数据，通信出现混乱。因此，打开有名管道时，</font><font style="color:#3b3b3b;">flags</font><font style="color:#3b3b3b;">只应为</font><font style="color:#3b3b3b;">O_WRONLY</font><font style="color:#3b3b3b;">或</font><font style="color:#3b3b3b;">O_RDONLY</font><font style="color:#3b3b3b;">。</font>

<font style="color:#3b3b3b;">内核为每个被进程打开的FIFO专用文件维护一个管道对象。当进程通过FIFO交换数据时，内核会在内部传递所有数据，不会将其写入文件系统。因此，/tmp/myfifo文件大小始终为0。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527835743-b8bdb1b4-ccce-43d3-a0c5-dcfc45722299.png)

<font style="color:#3b3b3b;">需要注意的是，文件详细信息最开头的字母</font><font style="color:#3b3b3b;">p</font><font style="color:#3b3b3b;">表示这个是一个有名管道文件。</font>

#### 图解

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763527856770-d7fe1c13-afcb-46f9-8f15-56e3009d93dc.png)

### 共享内存

#### shm_open()和shm_unlink()

<font style="color:#3b3b3b;">shm_open可以开启一块内存共享对象，我们可以像使用一般文件描述符一般使用这块内存对象。</font>

```cpp
#include <sys/mman.h>

/**
 * const char *name: 这是共享内存对象的名称，直接写一个文件名称，本身会保存在 /dev/shm 。名称必须是唯一的，以便不同进程可以定位同一个共享内存段。
 * 命名规则：必须是以正斜杠/开头，以\0结尾的字符串，中间可以包含若干字符，但不能有正斜杠
 * int oflag: 打开模式 二进制可拼接
 *      (1) O_CREAT：如果不存在则创建新的共享内存对象
 *      (2) O_EXCL：当与 O_CREAT 一起使用时，如果共享内存对象已经存在，则返回错误（避免覆盖现有对象）
 *      (3) O_RDONLY：以只读方式打开
 *      (4) O_RDWR：以读写方式打开
 *      (5) O_TRUNC 用于截断现有对象至0长度（只有在打开模式中包含 O_RDWR 时才有效）。
 * mode_t mode: 当创建新共享内存对象时使用的权限位，类似于文件的权限模式,一般0644即可
 * return: 成功执行,它将返回一个新的描述符;发生错误,返回值为 -1
*/
int shm_open(const char *name, int oflag, mode_t mode);

/**
 * 
 * 删除一个先前由 shm_open() 创建的命名共享内存对象。尽管这个函数被称为“unlink”，但它并没有真正删除共享内存段本身，而是移除了与共享内存对象关联的名称，使得通过该名称无法再打开共享内存。当所有已打开该共享内存段的进程关闭它们的描述符后，系统才会真正释放共享内存资源
 *
 * char *name: 要删除的共享内存对象名称
 * return: 成功返回0 失败返回-1
 */
int shm_unlink(const char *name);
```

#### truncate()和ftruncate()

<font style="color:#3b3b3b;">truncate</font><font style="color:#3b3b3b;">和</font><font style="color:#3b3b3b;">ftruncate</font><font style="color:#3b3b3b;">都可以将文件缩放到指定大小，二者的行为类似：如果文件被缩小，截断部分的数据丢失，如果文件空间被放大，扩展的部分均为</font><font style="color:#3b3b3b;">\0</font><font style="color:#3b3b3b;">字符。缩放前后文件的偏移量不会更改。缩放成功返回</font><font style="color:#3b3b3b;">0</font><font style="color:#3b3b3b;">，失败返回</font><font style="color:#3b3b3b;">-1</font><font style="color:#3b3b3b;">。</font>

<font style="color:#3b3b3b;">不同的是，前者需要指定路径，而后者需要提供文件描述符；ftruncate缩放的文件描述符可以是通过shm_open()开启的内存对象，而truncate缩放的文件必须是文件系统已存在文件，若文件不存在或没有权限则会失败。</font>

```cpp
#include <unistd.h>
#include <sys/types.h>

/**
 * 将指定文件扩展或截取到指定大小
 * 
 * char *path: 文件名 指定存在的文件即可 不需要打开
 * off_t length: 指定长度 单位字节
 * return: int 成功 0
 *             失败 -1
 */
int truncate(const char *path, off_t length);
/**
 *  将指定文件描述符扩展或截取到指定大小
 * 
 * int fd: 文件描述符 需要打开并且有写权限
 * off_t length: 指定长度 单位字节
 * return: int 成功 0
 *             失败 -1
 */
int ftruncate(int fd, off_t length);
```

#### mmap()

<font style="color:#3b3b3b;">mmap系统调用可以将一组设备或者文件映射到内存地址，我们在内存中寻址就相当于在读取这个文件指定地址的数据。父进程在创建一个内存共享对象并将其映射到内存区后，子进程可以正常读写该内存区，并且父进程也能看到更改。使用man 2 mmap查看该系统调用声明：</font>

```cpp
#include <sys/mman.h>

/**
 * 将文件映射到内存区域,进程可以直接对内存区域进行读写操作,就像操作普通内存一样,但实际上是对文件或设备进行读写,从而实现高效的 I/O 操作
 * 
 * void *addr: 指向期望映射的内存起始地址的指针,通常设为 NULL,让系统选择合适的地址
 * size_t length: 要映射的内存区域的长度,以字节为单位
 * int prot: 内存映射区域的保护标志,可以是以下标志的组合
 *          (1) PROT_READ: 允许读取映射区域
 *          (2) PROT_WRITE: 允许写入映射区域
 *          (3) PROT_EXEC: 允许执行映射区域
 *          (4) PROT_NONE: 页面不可访问
 * int flags：映射选项标志
 *          (1) MAP_SHARED: 映射区域是共享的,对映射区域的修改会影响文件和其他映射到同一区域的进程(一般使用共享)
 *          (2) MAP_PRIVATE: 映射区域是私有的,对映射区域的修改不会影响原始文件,对文件的修改会被暂时保存在一个私有副本中
 *          (3) MAP_ANONYMOUS: 创建一个匿名映射,不与任何文件关联
 *          (4) MAP_FIXED: 强制映射到指定的地址,如果不允许映射,将返回错误
 * int fd: 文件描述符,用于指定要映射的文件或设备,如果是匿名映射,则传入无效的文件描述符（例如-1）
 * off_t offset: 从文件开头的偏移量,映射开始的位置
 * return void*: (1) 成功时,返回映射区域的起始地址,可以像操作普通内存那样使用这个地址进行读写
 *               (2) 如果出错,返回 (void *) -1,并且设置 errno 变量来表示错误原因
 */
void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset);
/**
 * 用于取消之前通过 mmap() 函数建立的内存映射关系
 * 
 * void *addr: 这是指向之前通过 mmap() 映射的内存区域的起始地址的指针,这个地址必须是有效的,并且必须是 mmap() 返回的有效映射地址
 * size_t length: 这是要解除映射的内存区域的大小(以字节为单位),它必须与之前通过 mmap() 映射的大小一致
 * return: int 成功 0
 *             失败 -1
 */
int munmap(void *addr, size_t length);
```

#### 测试例程

<font style="color:#3b3b3b;">下面的程序展示了如何使用</font><font style="color:#3b3b3b;">mmap</font><font style="color:#3b3b3b;">在父子进程之间共享信息。</font>

（1）<font style="color:#3b3b3b;">创建shared_memory.c，写入以下内容</font>

```cpp
#include <fcntl.h>  // 定义 O_CREAT, O_RDWR 等标志
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>  // 核心头文件：包含 mmap, shm_open, shm_unlink 等声明
#include <sys/wait.h>  // wait 函数
#include <unistd.h>    // fork, getpid, ftruncate, close, sleep

int main() {
  // 定义指向共享内存区域的指针
  char* share;
  pid_t pid;

  // 构造共享内存对象的名称
  // POSIX共享内存名称必须以 '/' 开头
  char shmName[100] = {0};
  sprintf(shmName, "/letter%d", getpid());

  // 1. 创建或打开共享内存对象 (Shared Memory Object)
  // shm_open 返回一个文件描述符，类似于 open()
  // O_CREAT: 如果不存在则创建
  // O_RDWR: 以读写方式打开
  // 0644: 设定权限 (文件所有者读写，组和其他人只读)
  int fd;
  fd = shm_open(shmName, O_CREAT | O_RDWR, 0644);
  if (fd < 0) {
    perror(
        "共享内存对象开启失败");  // perror会自动加冒号和换行，字符串里一般不用加\n
    exit(EXIT_FAILURE);
  }

  // 2. 调整共享内存大小
  // 新创建的共享内存对象大小默认为 0，必须调用 ftruncate 扩容
  // 这里将其扩充为 100 字节
  ftruncate(fd, 100);

  // 3. 将共享内存对象映射到进程的虚拟地址空间
  // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t
  // offset); NULL: 让内核自动选择映射的起始地址 100: 映射长度 PROT_READ |
  // PROT_WRITE: 内存区域可读可写 MAP_SHARED:
  // 关键标志！表示对内存的修改会同步到底层对象，且其他映射该对象的进程可见（实现通信）
  // fd: 共享内存的文件描述符
  // 0: 偏移量，从头开始映射
  share = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  // 错误检查：mmap 失败返回 MAP_FAILED (通常是 (void*)-1)，而不是 NULL
  if (share == MAP_FAILED) {
    perror("共享内存对象映射到内存失败");
    exit(EXIT_FAILURE);
  }

  // 4. 关闭文件描述符
  // 映射建立后，fd 就不再需要了，关闭它不会影响已经建立的内存映射
  // 这也是一种良好的编程习惯，避免资源泄漏
  close(fd);

  // 5. 创建子进程
  pid = fork();

  if (pid == 0) {
    // ---------------- 子进程执行区域 ----------------

    // 向共享内存写入数据
    // 因为 share 已经被映射到共享内存，直接操作指针即可，像操作普通数组一样
    strcpy(share, "你是个好人!\n");
    printf("新学员(子进程 %d)完成回信!\n", getpid());

    // 注意：子进程执行完这里后，会继续向下执行 main 函数剩余部分
    // 包括最后的 shm_unlink。通常建议子进程在这里显式 exit(0);
  } else {
    // ---------------- 父进程执行区域 ----------------

    // 简单的同步：等待1秒，确保子进程已经把数据写进去了
    // 在生产环境中，通常会配合信号量(semaphore)或互斥锁使用，而不是 sleep
    sleep(1);

    // 直接读取共享内存中的数据
    printf("老学员(父进程 %d)看到新学员%d回信的内容: %s", getpid(), pid, share);

    // 等待子进程终止，回收僵尸进程
    wait(NULL);

    // 6. 解除内存映射
    // 进程结束时会自动解除，但显式调用是个好习惯
    int ret = munmap(share, 100);
    if (ret == -1) {
      perror("munmap");
      exit(EXIT_FAILURE);
    }
  }

  // 7. 删除共享内存对象
  // shm_unlink 类似于文件系统的 rm，将该名称从系统中移除
  // 注意：这不会立即销毁内存内容，直到所有映射该内存的进程都解除了映射 (munmap)
  // 或退出了 由于父子进程都会执行到这里，shm_unlink 会被调用两次。
  // 第一次成功，第二次会失败(因为已经删除了)，但不影响程序运行。
  shm_unlink(shmName);

  return 0;
}
```

（2）Makefile

```cpp
shared_memory: shared_memory.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）运行

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763528121086-38c4a733-2f9d-44b8-aef8-9382d07c4842.png)

#### 临时文件系统

<font style="color:#3b3b3b;">Linux</font><font style="color:#3b3b3b;">的临时文件系统（</font><font style="color:#3b3b3b;">tmpfs</font><font style="color:#3b3b3b;">）是一种基于内存的文件系统，它将数据存储在</font><font style="color:#3b3b3b;">RAM</font><font style="color:#3b3b3b;">或者在需要时部分使用交换空间（</font><font style="color:#3b3b3b;">swap</font><font style="color:#3b3b3b;">）。</font><font style="color:#3b3b3b;">tmpfs</font><font style="color:#3b3b3b;">访问速度快，但因为存储在内存，重启后数据清空，通常用于存储一些临时文件。</font>

<font style="color:#3b3b3b;">我们可以通过</font><font style="color:#3b3b3b;">df -h</font><font style="color:#3b3b3b;">查看当前操作系统已挂载的文件系统。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763528139516-d2295521-6bc1-47b6-a2f4-b34bc499e82c.png)

#### 内存共享对象在临时文件系统中的表示

<font style="color:#3b3b3b;">内存共享对象在临时文件系统中的表示位于</font><font style="color:#3b3b3b;">/dev/shm</font><font style="color:#3b3b3b;">目录下。</font>

<font style="color:#3b3b3b;">为看到共享对象在临时文件系统中的表示，我们修改程序，在创建后不销毁。</font>

（1）<font style="color:#3b3b3b;">创建shared_memory_block.c，写入以下内容</font>

```cpp
#include <fcntl.h>  // 定义 O_CREAT, O_RDWR 等标志
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>  // shm_open 函数的头文件
#include <sys/wait.h>
#include <unistd.h>  // getpid 函数

int main() {
  // 定义变量 (注意：share 和 pid 在此段代码中声明了但未使用)
  char* share;
  pid_t pid;

  // 准备存放共享内存名称的缓冲区
  char shmName[100] = {0};

  // 1. 构造唯一的共享内存名称
  // 格式："/letter" + "当前进程ID"
  // 例如：如果当前进程ID是 1234，名称就是 "/letter1234"
  // 注意：POSIX 共享内存名称通常要求以 '/' 开头
  sprintf(shmName, "/letter%d", getpid());

  // 打印生成的名称，方便你去 /dev/shm 目录下查找
  printf("shmName: %s\n", shmName);

  // 2. 创建共享内存对象
  // shm_open: 在内核中创建一个共享内存对象
  // 参数1: 名称
  // 参数2: O_CREAT (不存在则创建) | O_RDWR (读写模式)
  // 参数3: 0644 (权限设置：所有者读写，其他人只读)
  // 返回值: 成功返回文件描述符 fd，失败返回 -1
  int fd;
  fd = shm_open(shmName, O_CREAT | O_RDWR, 0644);

  if (fd < 0) {
    // 开启失败报错
    // 注意：perror会自动换行，字符串里其实不需要再加 '\n'
    perror("共享内存对象开启失败!\n");
    exit(EXIT_FAILURE);
  }

  // 3. 死循环 (重点)
  // 程序运行到这里会卡住，持续占用 CPU
  // 目的：保持进程存活，不关闭文件描述符，也不结束程序。
  // 这时你可以打开另一个终端窗口，输入 'ls -l /dev/shm'
  // 你应该能看到一个名为 letterXXXX 的文件，大小为 0 (因为还没 ftruncate)
  while (1)
    ;
  return 0;
}
```

（2）Makefile

```makefile
shared_memory_block: shared_memory_block.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

（3）执行

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763528204787-4a6ff220-6f82-46f8-bc5d-0a2381c93caf.png)

（4）查看/dev/shm目录

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763528212721-894cb9b6-9f26-4e21-bbea-5315a0abc2ac.png)

### 消息队列

#### 相关数据类型

##### <font style="color:#3b3b3b;">mqd_t</font>

<font style="color:#3b3b3b;">该数据类型定义在mqueue.h中，是用来记录消息队列描述符的。</font>

```cpp
typedef int mqd_t;
```

<font style="color:#3b3b3b;">实质上是int类型的别名。</font>

##### <font style="color:#3b3b3b;">struct mq_attr</font>

```cpp
/**
 * @brief 消息队列的属性信息
 * mq_flags 标记，对于mq_open，忽略它，因为这个标记是通过前者的调用传递的
 * mq_maxmgs 队列可以容纳的消息的最大数量
 * mq_msgsize 单条消息的最大允许大小，以字节为单位
 * mq_curmsgs 当前队列中的消息数量，对于mq_open，忽略它
 */
struct mq_attr {
long mq_flags;   /* Flags (ignored for mq_open()) */
long mq_maxmsg;  /* Max. # of messages on queue */
long mq_msgsize; /* Max. message size (bytes) */
long mq_curmsgs; /* # of messages currently in queue
                (ignored for mq_open()) */
};
```

##### struct timespec

```cpp
/**
 * @brief 时间结构体，提供了纳秒级的UNIX时间戳
 * tv_sec 秒
 * tv_nsec 纳秒
 */
struct timespec {
time_t tv_sec;        /* seconds */
long   tv_nsec;       /* nanoseconds */
};
```

#### 相关系统调用

##### mq_open()

```cpp
#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */
#include <mqueue.h>

/**
 * @brief 创建或打开一个已存在的POSIX消息队列，消息队列是通过名称唯一标识的。
 *
 * @param name 消息队列的名称
 * 命名规则：必须是以正斜杠/开头，以\0结尾的字符串，中间可以包含若干字符，但不能有正斜杠
 * @param oflag 指定消息队列的控制权限，必须也只能包含以下三者之一
 * O_RDONLY 打开的消息队列只用于接收消息
 * O_WRONLY 打开的消息队列只用于发送消息
 * O_RDWR 打开的消息队列可以用于收发消息
 * 可以与以下选项中的0至多个或操作之后作为oflag
 * O_CLOEXEC 设置close-on-exec标记，这个标记表示执行exec时关闭文件描述符
 * O_CREAT 当文件描述符不存在时创建它，如果指定了这一标记，需要额外提供mode和attr参数
 * O_EXCL 创建一个当前进程独占的消息队列，要同时指定O_CREAT，要求创建的消息队列不存在，否则将会失败，并提示错误EEXIST
 * O_NONBLOCK 以非阻塞模式打开消息队列，如果设置了这个选项，在默认情况下收发消息发生阻塞时，会转而失败，并提示错误EAGAIN
 * @param mode 每个消息队列在mqueue文件系统对应一个文件，mode是用来指定消息队列对应文件的权限的
 * @param attr 属性信息，如果为NULL，则队列以默认属性创建

* @return mqd_t 成功则返回消息队列描述符，失败则返回(mqd_t)-1，同时设置errno以指明错误原因
*/
mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);

/**
 * @brief 当oflag没有包含O_CREAT时方可调用
 *
 * @param name 同上
 * @param oflag 同上
 * @return mqd_t 同上
 */
mqd_t mq_open(const char *name, int oflag);
```

##### mq_timedsend()

```cpp
#include <time.h>
#include <mqueue.h>

/**
 * @brief 将msg_ptr指向的消息追加到消息队列描述符mqdes指向的消息队列的尾部。如果消息队列已满，默认情况下，调用阻塞直至有充足的空间允许新的消息入队，或者达到abs_timeout指定的等待时间节点，或者调用被信号处理函数打断。需要注意的是，正如上文提到的，如果在mq_open时指定了O_NONBLOCK标记，则转而失败，并返回错误EAGAIN。
 * 
 * @param mqdes 消息队列描述符
 * @param msg_ptr 指向消息的指针
 * @param msg_len msg_ptr指向的消息长度，不能超过队列的mq_msgsize属性指定的队列最大容量，长度为0的消息是被允许的
 * @param msg_prio 一个非负整数，指定了消息的优先级，消息队列中的数据是按照优先级降序排列的，如果新旧消息的优先级相同，则新的消息排在后面。
 * @param abs_timeout 指向struct timespec类型的对象，指定了阻塞等待的最晚时间。如果消息队列已满，且abs_timeout指定的时间节点已过期，则调用立即返回。
 * @return int 成功返回0，失败返回-1，同时设置errno以指明错误原因。
 */
int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec *abs_timeout);
```

##### mq_timedreceive()

```cpp
#include <time.h>
#include <mqueue.h>

/**
 * @brief 从消息队列中取走最早入队且权限最高的消息，将其放入msg_ptr指向的缓存中。如果消息队列为空，默认情况下调用阻塞，此时的行为与mq_timedsend同理。
 * 
 * @param mqdes 消息队列描述符
 * @param msg_ptr 接收消息的缓存
 * @param msg_len msg_ptr指向的缓存区的大小，必须大于等于mq_msgsize属性指定的队列单条消息最大字节数
 * @param msg_prio 如果不为NULL，则用于接收接收到的消息的优先级 
 * @param abs_timeout 阻塞时等待的最晚时间节点，同mq_timedsend
 * @return ssize_t 成功则返回接收到的消息的字节数，失败返回-1，并设置errno指明错误原因
 */
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio, const struct timespec *abs_timeout);
```

##### mq_unlink()

```cpp
#include <mqueue.h>

/**
 * @brief 清除name对应的消息队列，mqueue文件系统中的对应文件被立即清除。消息队列本身的清除必须等待所有指向该消息队列的描述符全部关闭之后才会发生。
 * 
 * @param name 消息队列名称
 * @return int 成功返回0，失败返回-1，并设置errno指明错误原因
 */
int mq_unlink(const char *name);
```

##### clock_gettime()

```cpp
#include <time.h>

/**
 * @brief 获取以struct timespec形式表示的clockid指定的时钟
 * 
 * @param clockid 特定时钟的标识符，常用的是CLOCK_REALTIME，表示当前真实时间的时钟
 * @param tp 用于接收时间信息的缓存
 * @return int 成功返回0，失败返回-1，同时设置errno以指明错误原因
 */
int clock_gettime(clockid_t clockid, struct timespec *tp);
```

#### 父子进程间通信测试例程

##### 创建father_son_mq_test.c

```cpp
#include <fcntl.h>   // 定义 O_CREAT, O_RDWR 等标志
#include <mqueue.h>  // 消息队列核心头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  // 权限模式定义
#include <time.h>      // struct timespec, clock_gettime
#include <unistd.h>    // fork, sleep, close

int main(int argc, char const* argv[]) {
  // ---------------- 1. 定义消息队列属性 ----------------
  struct mq_attr attr;
  // 核心参数: 队列中最大允许存放的消息数量
  attr.mq_maxmsg = 10;
  // 核心参数: 每条消息的最大字节数
  attr.mq_msgsize = 100;
  // 以下参数在 mq_open 创建队列时会被忽略，仅用于 mq_getattr
  attr.mq_flags = 0;    // 0 表示阻塞模式
  attr.mq_curmsgs = 0;  // 当前消息数

  // 定义消息队列名称 (必须以 '/' 开头)
  char* mq_name = "/father_son_mq";

  // ---------------- 2. 创建或打开消息队列 ----------------
  // O_RDWR: 读写模式打开 (因为父进程要写，子进程要读)
  // O_CREAT: 如果不存在则创建
  // 0664: 权限设置 (所有者读写，组读写，其他人只读)
  mqd_t mqdes = mq_open(mq_name, O_RDWR | O_CREAT, 0664, &attr);

  if (mqdes == (mqd_t)-1) {
    perror("mq_open");
    exit(EXIT_FAILURE);
  }

  // ---------------- 3. 创建子进程 ----------------
  // fork 后，子进程会继承父进程打开的文件描述符 mqdes
  // 因此父子进程可以直接使用同一个描述符操作队列
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  // ---------------- 4. 子进程逻辑 (消费者) ----------------
  if (pid == 0) {
    char read_buf[100];
    struct timespec time_info;

    // 循环接收 10 次消息
    for (size_t i = 0; i < 10; i++) {
      // 每次接收前清空缓冲区，防止数据残留
      memset(read_buf, 0, 100);

      // 设置超时时间: 获取当前时间 + 15秒
      // clock_gettime(0, ...) 中的 0 等同于 CLOCK_REALTIME
      clock_gettime(CLOCK_REALTIME, &time_info);
      time_info.tv_sec += 15;

      // 接收消息
      // 如果队列为空，进程会阻塞，直到有消息到达或超过 15秒
      // 参数 NULL 表示不关心消息的优先级
      if (mq_timedreceive(mqdes, read_buf, 100, NULL, &time_info) == -1) {
        perror("mq_timedreceive");
      }

      printf("[子进程] 接收到数据: %s\n", read_buf);
    }

    // ---------------- 5. 父进程逻辑 (生产者) ----------------
  } else {
    char send_buf[100];
    struct timespec time_info;

    // 循环发送 10 次消息
    for (size_t i = 0; i < 10; i++) {
      // 清空发送缓冲区
      memset(send_buf, 0, 100);
      // 格式化字符串写入 buffer
      sprintf(send_buf, "父进程的第%d次发送消息", (int)(i + 1));

      // 设置超时时间: 当前时间 + 5秒
      clock_gettime(CLOCK_REALTIME, &time_info);
      time_info.tv_sec += 5;

      // 发送消息
      // 如果队列已满 (超过 mq_maxmsg 10条)，进程会阻塞，直到有空间或超时
      // 参数 0 表示默认优先级
      if (mq_timedsend(mqdes, send_buf, strlen(send_buf), 0, &time_info) ==
          -1) {
        perror("mq_timedsend");
      }

      printf("[父进程] 发送一条消息, 休眠1s...\n");

      // 休眠 1 秒，模拟生产数据的耗时
      // 这也给了子进程读取数据的时间
      sleep(1);
    }

    // 等待子进程退出 (防止子进程变成僵尸进程)
    // 虽然原代码没写 wait，但在父子协作中通常建议加上 wait(NULL);
  }

  // ---------------- 6. 资源清理 ----------------

  // 无论是父进程还是子进程，退出前都应关闭文件描述符
  // 这只是断开当前进程与队列的连接
  close(mqdes);

  // 只有父进程负责“物理删除”消息队列
  // 确保所有进程都不再使用后，从系统中移除该队列名
  if (pid > 0) {
    mq_unlink(mq_name);
    printf("[父进程] 消息队列已清理，程序结束。\n");
  }

  return 0;
}
```

##### Makefile

```makefile
father_son_mq_test: father_son_mq_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

##### 执行结果如下

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763528506975-af0d4ca7-69e6-4914-bb8b-376380afd409.png)

<font style="color:#3b3b3b;">子进程正确接收了父进程发送的数据。</font>

#### 非父子进程通信案例

<font style="color:#3b3b3b;">我们创建两个进程：生产者和消费者，前者从控制台接收数据并写入消息队列，后者从消息队列接收数据并打印到控制台。</font>

##### <font style="color:#3b3b3b;">创建producer.c</font>

```cpp
#include <mqueue.h>     // 消息队列核心头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>       // 时间结构体 struct timespec, clock_gettime
#include <unistd.h>     // read, close

int main() {
    // 定义消息队列的名称
    // 注意：POSIX 消息队列名称通常要求以 '/' 开头
    char* mq_name = "/p_c_mq";

    // 定义消息队列的属性结构体
    struct mq_attr attr;

    // 0: 表示默认行为（阻塞模式），即如果队列满了，发送会等待
    attr.mq_flags = 0;
    // 队列中允许的最大消息数量 (Linux默认限制通常较小，如10)
    attr.mq_maxmsg = 10;
    // 每条消息的最大字节数
    attr.mq_msgsize = 100;
    // 当前队列中的消息数（此字段在 mq_open 创建时会被忽略，仅用于 mq_getattr 获取状态）
    attr.mq_curmsgs = 0;

    // 1. 创建或打开消息队列
    // mq_open 返回一个消息队列描述符 (mqd_t)
    // O_CREAT: 如果不存在则创建
    // O_WRONLY: 以只写模式打开（生产者只需要写）
    // 0666: 权限设置（rw-rw-rw-）
    // &attr: 传入属性设置（仅在创建新队列时生效）
    mqd_t mqdes = mq_open(mq_name, O_CREAT | O_WRONLY, 0666, &attr);

    // 错误检查：mq_open 失败返回 (mqd_t)-1
    if (mqdes == (mqd_t)-1) {
        perror("mq_open");
        // 在这里通常应该 exit，否则后续操作会崩溃，但为了保持原逻辑未添加
    }

    char writeBuf[100];
    struct timespec time_info; // 用于设置超时时间

    while (1) {
        // 清空写缓冲区，防止残留上次的数据
        // 这是一个好习惯，确保字符串以 \0 结尾
        memset(writeBuf, 0, 100);

        // 2. 从命令行标准输入 (STDIN_FILENO, 即 0) 读取数据
        // 程序会在这里阻塞，等待用户输入并回车
        ssize_t read_count = read(0, writeBuf, 100);

        if (read_count == -1) {
            perror("read");
            continue; // 读取出错则重试
        } 
        // 注意：这里原来的代码有个空的 else if (read_count == 0)，逻辑上是多余的，
        // 因为下面还有一个专门处理 read_count == 0 的判断块。

        // 3. 设置超时时间
        // mq_timedsend 需要的是【绝对时间】（Absolute Time），而不是相对时长。
        // 必须先获取当前时间，再加上想要等待的秒数。
        
        // 获取当前系统实时时间
        clock_gettime(CLOCK_REALTIME, &time_info);
        // 在当前时间基础上加 5 秒
        // 意味着：如果队列满了，发送操作最多阻塞等待 5 秒，超时则返回错误
        time_info.tv_sec += 5;

        // 4. 处理 EOF (Ctrl+D)
        // 当用户在终端按下 Ctrl+D 时，read 返回 0
        if (read_count == 0) {
            printf("Received EOF, exit...\n");
            
            // 准备发送一个特殊的结束标志
            // EOF 是一个宏（通常是 -1），强制转为 char 后发送给消费者
            // 消费者收到这个特殊字符就知道该停止了
            char eof = EOF; 
            
            // 尝试发送结束信号
            if (mq_timedsend(mqdes, &eof, 1, 0, &time_info) == -1) {
                perror("mq_timedsend");
            }
            break; // 跳出循环，结束程序
        }

        // 5. 正常发送数据
        // 参数说明：
        // mqdes: 队列描述符
        // writeBuf: 数据指针
        // strlen(writeBuf): 数据长度 (注意：read读入的数据通常包含换行符)
        // 0: 消息优先级 (Priority)，非负整数，数值越大优先级越高。0是默认。
        // &time_info: 绝对超时时间
        if (mq_timedsend(mqdes, writeBuf, strlen(writeBuf), 0, &time_info) == -1) {
            perror("mq_timesend"); // 发送失败（例如队列满且超时、或队列不存在）
        }
        
        printf("从命令行接收到数据，已发送至消费者端\n");
    }

    // 6. 关闭消息队列描述符
    // 注意：在标准 POSIX 中应使用 mq_close(mqdes)，
    // 虽然在 Linux 上 close(fd) 也能工作（因为 mqd_t 是文件描述符），但为了移植性建议用 mq_close
    close(mqdes); 
    // 建议改为: mq_close(mqdes);

    // 提示：mq_unlink 用于物理删除队列名字。
    // 通常由消费者（最后一个使用者）来执行，或者在完全不再需要队列时执行。
    // 如果这里调用 unlink，可能导致消费者还没读完队列就消失了。

    return 0;
}
```

##### 创建consumer.c

```cpp
#include <time.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
    char *mq_name = "/p_c_mq";

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    attr.mq_curmsgs = 0;

    // 创建或打开消息队列
    mqd_t mqdes = mq_open(mq_name, O_CREAT | O_RDONLY, 0666, &attr);

    if (mqdes == -1)
    {
        perror("mq_open");
    }

    char readBuf[100];
    struct timespec time_info;

    while (1)
    {
        memset(readBuf, 0, 100);

        // 获取1天后的time_spec结构对象，目的是在测试期间使得消费者一直等待生产者发送的数据
        clock_gettime(CLOCK_REALTIME, &time_info);
        time_info.tv_sec += 86400;

        // 接收数据
        if (mq_timedreceive(mqdes, readBuf, 100, NULL, &time_info) == -1)
        {
            perror("mq_timedreceive");
        }

        // 如果收到生产者发送的EOF，则结束进程
        if (readBuf[0] == EOF)
        {
            printf("接收到生产者的终止信号，准备退出...\n");
            break;
        }
        // 如果没有收到EOF，将接收到的数据打印到标准输出
        printf("接收到来自于生产者的数据\n%s", readBuf);
    }

    // 释放消息队列描述符
    close(mqdes);

    // mq_unlink 只应调用一次
    // 清除消息队列
    mq_unlink(mq_name);
}

```

##### Makefile

<font style="color:#3b3b3b;">在伪目标中添加p_c_mq和pc_mq_clean，如下。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763528775334-06e4a1e7-9f7c-455d-952c-0c0728317b13.png)

<font style="color:#3b3b3b;">补充以下内容</font>

```makefile
producer: producer.c
    $(CC) -o $@ $^

cosumer: consumer.c
    $(CC) -o $@ $^

p_c_mq: producer consumer

pc_mq_clean:
    -rm ./producer ./consumer
```

#### 消息队列在mqueue文件系统的表示

<font style="color:#3b3b3b;">我们启动producer和consumer，然后查看/dev/mqueue目录，如下。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763528811969-edbf95df-974f-41bf-9ea0-3323af364c9d.png)

<font style="color:#3b3b3b;">终止消费和生产者后，上述文件被清除。</font>

#### 消息队列通信方向的讨论

<font style="color:#3b3b3b;">我们可以通过设置POSIX消息队列的模式为O_RDWR，使它可以用于收发数据，从技术上讲，单条消息队列可以用于双向通信，但是这会导致消息混乱，无法确定队列中的数据是本进程写入的还是读取的，因此，不会这么做，通常单条消息队列只用于单向通信。为了实现全双工通信，我们可以使用两条消息队列，分别负责两个方向的通信。类似于管道。</font>

## 信号

### 信号简介

<font style="color:#3b3b3b;">在</font><font style="color:#3b3b3b;">Linux</font><font style="color:#3b3b3b;">中，信号是一种用于通知进程发生了某种事件的机制。信号可以由内核、其他进程或者通过命令行工具发送给目标进程。</font><font style="color:#3b3b3b;">Linux</font><font style="color:#3b3b3b;">系统中有多种信号，每种信号都用一个唯一的整数值来表示，例如，常见的信号包括：</font>

（1）<font style="color:#3b3b3b;">SIGINT</font><font style="color:#3b3b3b;">（</font><font style="color:#3b3b3b;">2</font><font style="color:#3b3b3b;">）：这是当用户在终端按下</font><font style="color:#3b3b3b;">Ctrl+C</font><font style="color:#3b3b3b;">时发送给前台进程的信号，通常用于请求进程终止。</font>

（2）<font style="color:#3b3b3b;">SIGKILL</font><font style="color:#3b3b3b;">（</font><font style="color:#3b3b3b;">9</font><font style="color:#3b3b3b;">）：这是一种强制终止进程的信号，它会立即终止目标进程，且不能被捕获或忽略。</font>

（3）<font style="color:#3b3b3b;">SIGTERM</font><font style="color:#3b3b3b;">（</font><font style="color:#3b3b3b;">15</font><font style="color:#3b3b3b;">）：这是一种用于请求进程终止的信号，通常由系统管理员或其他进程发送给目标进程。</font>

（4）<font style="color:#3b3b3b;">SIGUSR1</font><font style="color:#3b3b3b;">（</font><font style="color:#3b3b3b;">10</font><font style="color:#3b3b3b;">）和</font><font style="color:#3b3b3b;">SIGUSR2</font><font style="color:#3b3b3b;">（</font><font style="color:#3b3b3b;">12</font><font style="color:#3b3b3b;">）：这两个信号是用户自定义的信号，可以由应用程序使用。</font>

（5）<font style="color:#3b3b3b;">SIGSEGV</font><font style="color:#3b3b3b;">（</font><font style="color:#3b3b3b;">11</font><font style="color:#3b3b3b;">）：这是一种表示进程非法内存访问的信号，通常是由于进程尝试访问未分配的内存或者试图执行非法指令而导致的。</font>

（6）<font style="color:#3b3b3b;">SIGALRM</font><font style="color:#3b3b3b;">（</font><font style="color:#3b3b3b;">14</font><font style="color:#3b3b3b;">）：这是一个定时器信号，通常用于在一定时间间隔后向目标进程发送信号。</font>

<font style="color:#3b3b3b;">每种信号都有其特定的含义和行为，进程可以通过注册信号处理函数来捕获信号并执行相应的操作，例如终止进程、忽略信号或者执行特定的处理逻辑。如果想查看所有的Linux信号，请执行kill -l指令，会得到以下反馈：</font>

```cpp
1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL       5) SIGTRAP
 6) SIGABRT      7) SIGBUS       8) SIGFPE       9) SIGKILL     10) SIGUSR1
11) SIGSEGV     12) SIGUSR2     13) SIGPIPE     14) SIGALRM     15) SIGTERM
16) SIGSTKFLT   17) SIGCHLD     18) SIGCONT     19) SIGSTOP     20) SIGTSTP
21) SIGTTIN     22) SIGTTOU     23) SIGURG      24) SIGXCPU     25) SIGXFSZ
26) SIGVTALRM   27) SIGPROF     28) SIGWINCH    29) SIGIO       30) SIGPWR
31) SIGSYS      34) SIGRTMIN    35) SIGRTMIN+1  36) SIGRTMIN+2  37) SIGRTMIN+3
38) SIGRTMIN+4  39) SIGRTMIN+5  40) SIGRTMIN+6  41) SIGRTMIN+7  42) SIGRTMIN+8
43) SIGRTMIN+9  44) SIGRTMIN+10 45) SIGRTMIN+11 46) SIGRTMIN+12 47) SIGRTMIN+13
48) SIGRTMIN+14 49) SIGRTMIN+15 50) SIGRTMAX-14 51) SIGRTMAX-13 52) SIGRTMAX-12
53) SIGRTMAX-11 54) SIGRTMAX-10 55) SIGRTMAX-9  56) SIGRTMAX-8  57) SIGRTMAX-7
58) SIGRTMAX-6  59) SIGRTMAX-5  60) SIGRTMAX-4  61) SIGRTMAX-3  62) SIGRTMAX-2
63) SIGRTMAX-1  64) SIGRTMAX
```

### 信号处理例程

<font style="color:#3b3b3b;">我们可以通过signal系统调用注册信号处理函数：</font>

```cpp
#include <signal.h>

// 信号处理函数声明
typedef void (*sighandler_t)(int);

/**
 *  signal系统调用会注册某一信号对应的处理函数。如果注册成功，当进程收到这一信号时，将不会调用默认的处理函数，而是调用这里的自定义函数
 * 
 * int signum: 要处理的信号
 * sighandler_t handler: 当收到对应的signum信号时，要调用的函数
 * return: sighandler_t 返回之前的信号处理函数，如果错误会返回SEG_ERR
 */
sighandler_t signal(int signum, sighandler_t handler);
```

<font style="color:#3b3b3b;">下面的例子简单演示了如何处理收到的信号。</font>

#### 创建signal_test.c，写入以下内容

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// 定义信号处理函数
void sigint_handler(int signum) {
    printf("\n收到%d信号,停止程序!\n",signum);
    exit(signum);
}
int main() {
    // 注册SIGINT信号处理函数 收到ctrl+c信号之后不执行默认的函数,而是执行新的注册函数
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("注册新的信号处理函数失败\n");
        return 1;
    }
   
    // 无限循环等待信号
    while (1) {
        sleep(1);
        printf("你好,在吗?\n");
    }
    return 0;
}
```

#### Makefile

```makefile
signal_test: signal_test.c
    -$(CC) -o $@ $^
    -./$@
    -rm ./$@
```

#### 运行

<font style="color:#3b3b3b;">程序运行后会以</font><font style="color:#3b3b3b;">1s</font><font style="color:#3b3b3b;">的间隔打印“你好，在吗？”。</font>

<font style="color:#3b3b3b;">光标移动到终端，按下Ctrl+C，结果如下。</font>

![](https://cdn.nlark.com/yuque/0/2025/png/33636091/1763528970125-b3fcbf05-34ff-4c1c-985e-ad41d6991dff.png)

<font style="color:#3b3b3b;">信号处理函数已被替换。</font>

<font style="color:#3b3b3b;"></font>

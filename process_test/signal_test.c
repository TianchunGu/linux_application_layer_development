#include <signal.h>  // 核心头文件: 定义了信号宏(如 SIGINT) 和 signal 函数
#include <stdio.h>   // 用于 printf, perror
#include <stdlib.h>  // 用于 exit 函数
#include <unistd.h>  // 用于 sleep 函数

/**
 * 自定义信号处理函数 (Signal Handler)
 * 当进程接收到特定信号时，操作系统会暂停主程序的执行，转而调用这个函数
 * * @param signum 接收到的信号编号 (例如 SIGINT 对应的值通常是 2)
 */
void sigint_handler(int signum) {
  // 打印提示信息，告知用户信号已被捕捉
  printf("\n收到%d信号 (SIGINT), 停止程序!\n", signum);

  // 正常退出程序
  // 参数 signum 作为退出状态码返回给操作系统
  // 在 Shell 中可以通过 'echo $?' 查看这个值
  exit(signum);
}

int main() {
  // ---------------- 1. 注册信号处理函数 ----------------

  // signal 函数用于改变信号的行为
  // 参数1 (SIGINT): 我们要捕捉的信号。代表 "Interrupt" (中断)，通常由用户按下
  // Ctrl+C 触发 参数2 (sigint_handler): 当信号发生时，要调用的回调函数指针
  // 返回值: 成功返回之前的处理函数指针，失败返回 SIG_ERR

  // 这里的逻辑是：告诉内核，“如果有人对我发 SIGINT，别直接杀掉我，去执行
  // sigint_handler 函数”
  if (signal(SIGINT, sigint_handler) == SIG_ERR) {
    // 如果注册失败 (极少发生，除非信号不可被捕捉，如 SIGKILL)
    perror("注册新的信号处理函数失败");  // perror 会输出错误原因
    return 1;
  }

  // ---------------- 2. 模拟主程序运行 ----------------

  // 无限循环，保持程序存活，等待用户操作
  while (1) {
    // sleep(1): 让程序挂起(暂停) 1秒
    // 这既能模拟耗时任务，也能避免死循环占用 100% CPU 资源
    sleep(1);

    // 打印信息证明程序正在正常运行
    printf("你好,在吗?\n");
  }

  return 0;
}
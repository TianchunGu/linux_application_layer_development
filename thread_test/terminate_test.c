#include <math.h>
#include <pthread.h>  // 线程核心头文件: pthread_create, pthread_join, pthread_exit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 定义结果结构体
// 用途：因为 pthread_exit 只能返回一个 void* 指针，
// 如果想返回多个数据（如字符串及其长度），需要封装在一个结构体中。
typedef struct Result {
  char* p;  // 指向结果字符串的指针
  int len;  // 字符串长度
} Result;

/**
 * 红玫瑰线程函数
 * * void *argv: 通用指针参数，由 pthread_create 传递进来。
 * 这里传递的是一个 char 指针，代表结束线程的指令字符。
 * return: void* 返回值指针，会被 pthread_join 捕获。
 */
void* red_thread(void* argv) {
  // 1. 在堆区(Heap)申请内存存放结果结构体
  // 重要：不能使用局部变量（栈内存），因为线程结束后栈内存会被销毁，
  // 主线程访问时会变成悬空指针导致崩溃。
  Result* result = malloc(sizeof(Result));

  // 将 void* 强转回 char* 并取值，获取代号 (例如 'r')
  char code = *((char*)argv);

  // 申请输入缓冲区
  char* ans = malloc(101);

  while (1) {
    // 尝试从标准输入读取字符串
    // ⚠️ 注意：这里存在线程竞争。白玫瑰线程也在同时读取 stdin。
    // 用户输入的内容可能被红玫瑰读走，也可能被白玫瑰读走。
    fgets(ans, 100, stdin);

    // 检查输入的第一个字符是否匹配代号
    if (ans[0] == code) {
      // --- 接收到了对应的信息 ('r') ---

      // 释放输入缓冲区，避免内存泄漏
      free(ans);

      printf("红玫瑰离开了!\n");

      // 准备返回值
      // strdup 会在堆上分配内存并复制字符串，记得在 main 中释放
      char* redAns = strdup("红玫瑰独自去了纽约.\n");
      result->p = redAns;
      result->len = strlen(redAns);

      // 结束当前线程，并将 result 指针作为返回值传递给主线程
      // 相当于 return result;
      pthread_exit((void*)result);
    } else {
      // 如果被红玫瑰读到了但不是 'r' (比如读到了 'w' 或回车)
      printf("红玫瑰还在等你!\n");
    }
  }
}

/**
 * 白玫瑰线程函数
 * 逻辑与红玫瑰完全一致，只是处理的代号和返回的字符串不同
 */
void* white_thread(void* argv) {
  // 在堆上申请结果对象的内存
  Result* result = malloc(sizeof(Result));
  char code = *((char*)argv);

  char* ans = malloc(101);
  while (1) {
    // 从标准输入读取
    fgets(ans, 100, stdin);

    if (ans[0] == code) {
      free(ans);
      printf("白玫瑰离开了!\n");

      // 准备返回数据
      char* whiteAns = strdup("白玫瑰独自去了伦敦.\n");
      result->p = whiteAns;
      result->len = strlen(whiteAns);

      // 退出线程并返回结果
      pthread_exit((void*)result);
    } else {
      printf("白玫瑰还在等你!\n");
    }
  }
}

int main() {
  // 定义线程 ID 变量
  pthread_t pid_red;
  pthread_t pid_white;

  // 定义触发退出的代号
  char red_code = 'r';
  char white_code = 'w';

  // 定义指针用于接收子线程返回的结构体
  Result* red_result = NULL;
  Result* white_result = NULL;

  // ---------------- 1. 创建线程 ----------------

  // 创建红玫瑰线程
  // 参数1: 线程ID指针
  // 参数2: 线程属性 (NULL 为默认)
  // 参数3: 线程执行的函数
  // 参数4: 传递给函数的参数 (这里传了字符变量的地址)
  pthread_create(&pid_red, NULL, red_thread, &red_code);

  // 创建白玫瑰线程
  pthread_create(&pid_white, NULL, white_thread, &white_code);

  // ---------------- 2. 等待并回收红玫瑰 ----------------

  // pthread_join 是阻塞函数。
  // 主线程会停在这里，直到 pid_red 线程执行了 pthread_exit 才会继续往下走。
  // 参数2 (void **): 用于接收线程返回的 void* 指针。
  pthread_join(pid_red, (void**)&red_result);

  // 代码执行到这里，说明红玫瑰已经结束了
  printf("红玫瑰故事结局: %s\n", red_result->p);

  // 释放内存：先释放结构体里的字符串，再释放结构体本身
  free(red_result->p);
  free(red_result);

  // ---------------- 3. 等待并回收白玫瑰 ----------------

  // 注意：如果红玫瑰一直没结束，主线程会卡在上面那行 join，
  // 即使白玫瑰已经结束了，主线程也无法处理白玫瑰的结果，直到红玫瑰先结束。
  pthread_join(pid_white, (void**)&white_result);

  printf("白玫瑰故事结局: %s\n", white_result->p);

  // 释放内存
  free(white_result->p);
  free(white_result);

  return 0;
}
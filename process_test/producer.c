#include <mqueue.h>  // 消息队列核心头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    // 时间结构体 struct timespec, clock_gettime
#include <unistd.h>  // read, close

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
  // 当前队列中的消息数（此字段在 mq_open 创建时会被忽略，仅用于 mq_getattr
  // 获取状态）
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
  struct timespec time_info;  // 用于设置超时时间

  while (1) {
    // 清空写缓冲区，防止残留上次的数据
    // 这是一个好习惯，确保字符串以 \0 结尾
    memset(writeBuf, 0, 100);

    // 2. 从命令行标准输入 (STDIN_FILENO, 即 0) 读取数据
    // 程序会在这里阻塞，等待用户输入并回车
    ssize_t read_count = read(0, writeBuf, 100);

    if (read_count == -1) {
      perror("read");
      continue;  // 读取出错则重试
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
      break;  // 跳出循环，结束程序
    }

    // 5. 正常发送数据
    // 参数说明：
    // mqdes: 队列描述符
    // writeBuf: 数据指针
    // strlen(writeBuf): 数据长度 (注意：read读入的数据通常包含换行符)
    // 0: 消息优先级 (Priority)，非负整数，数值越大优先级越高。0是默认。
    // &time_info: 绝对超时时间
    if (mq_timedsend(mqdes, writeBuf, strlen(writeBuf), 0, &time_info) == -1) {
      perror("mq_timesend");  // 发送失败（例如队列满且超时、或队列不存在）
    }

    printf("从命令行接收到数据，已发送至消费者端\n");
  }

  // 6. 关闭消息队列描述符
  // 注意：在标准 POSIX 中应使用 mq_close(mqdes)，
  // 虽然在 Linux 上 close(fd) 也能工作（因为 mqd_t
  // 是文件描述符），但为了移植性建议用 mq_close
  close(mqdes);
  // 建议改为: mq_close(mqdes);

  // 提示：mq_unlink 用于物理删除队列名字。
  // 通常由消费者（最后一个使用者）来执行，或者在完全不再需要队列时执行。
  // 如果这里调用 unlink，可能导致消费者还没读完队列就消失了。

  return 0;
}

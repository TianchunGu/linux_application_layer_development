#include <mqueue.h>  // 消息队列核心头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    // struct timespec, clock_gettime
#include <unistd.h>  // close

int main() {
  // 队列名称，必须与生产者保持一致
  char* mq_name = "/p_c_mq";

  // 定义队列属性
  struct mq_attr attr;
  attr.mq_flags = 0;      // 0 表示阻塞模式 (默认)
  attr.mq_maxmsg = 10;    // 队列中最大消息数
  attr.mq_msgsize = 100;  // 每条消息的最大字节数
  attr.mq_curmsgs = 0;    // 当前消息数 (只读，创建时忽略)

  // 1. 打开消息队列
  // O_CREAT: 如果队列不存在则创建 (防止先启动消费者报错)
  // O_RDONLY: 消费者只需要读取权限
  // 0666: 权限设置
  mqd_t mqdes = mq_open(mq_name, O_CREAT | O_RDONLY, 0666, &attr);

  if (mqdes == (mqd_t)-1) {
    perror("mq_open");
    exit(EXIT_FAILURE);  // 建议出错直接退出
  }

  // 准备接收缓冲区
  // 注意：这个缓冲区大小必须 >= 队列属性中的 mq_msgsize (这里是100)
  // 否则 mq_receive 会失败并报错 EMSGSIZE
  char readBuf[100];
  struct timespec time_info;

  while (1) {
    // 每次接收前清空缓冲区
    // 既能清除上一轮的残留数据，也能保证字符串默认有 '\0' 结束符
    memset(readBuf, 0, 100);

    // 2. 设置超时时间 (绝对时间)
    // clock_gettime 获取当前时间
    clock_gettime(CLOCK_REALTIME, &time_info);
    // 加上 86400 秒 (即 24 小时)
    // 这里意图是模拟“一直等待”，只要 1 天内有数据发过来就能收到
    // 如果超过 1 天还没收到，mq_timedreceive 才会返回超时错误
    time_info.tv_sec += 86400;

    // 3. 接收数据
    // 参数:
    // mqdes: 队列描述符
    // readBuf: 接收数据的指针
    // 100: 缓冲区大小 (必须 >= mq_msgsize)
    // NULL: 不关心消息优先级，若关心可传 int* 指针接收优先级
    // &time_info: 绝对超时时间
    // 返回值: 成功返回接收到的字节数，失败返回 -1
    if (mq_timedreceive(mqdes, readBuf, 100, NULL, &time_info) == -1) {
      perror("mq_timedreceive");
      // 继续循环还是退出取决于业务逻辑，这里选择继续尝试
      continue;
    }

    // 4. 检查结束标志
    // 生产者发送了一个 char 类型的 EOF (通常值为 -1)
    // 如果收到这个特定字符，说明生产者已经不再发送数据了
    if (readBuf[0] == EOF) {
      printf("接收到生产者的终止信号，准备退出...\n");
      break;
    }

    // 5. 处理正常数据
    // 直接打印接收到的字符串
    // 由于生产者发送时带了换行符，或者 printf
    // 格式串里处理换行，这里直接输出即可
    printf("接收到来自于生产者的数据\n%s", readBuf);
  }

  // 6. 关闭消息队列描述符
  // 只是关闭当前进程与队列的连接，队列内容依然保留在内核中
  // 建议使用 POSIX 标准的 mq_close(mqdes);
  close(mqdes);

  // 7. 删除消息队列 (Unlink)
  // 这一步非常重要！
  // mq_unlink 会将队列名从系统中移除。
  // 只有当所有进程都关闭了该队列，且调用了
  // unlink，队列占用的内存资源才会被内核真正回收。
  // 通常由“最后一个使用者”或“管理者”来执行这一步。
  mq_unlink(mq_name);

  return 0;
}
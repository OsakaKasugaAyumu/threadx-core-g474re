# ThreadX 单核源码导读

**ThreadX 6.2.0 · Cortex-M4F · GNU Arm GCC · NUCLEO-G474RE**

用同一个 8 线程 Demo 看懂 ThreadX 如何切换线程、传递消息，以及这些行为在 NUCLEO-G474RE 上如何验证。读源码不需要接开发板。

**[打开中文阅读首页 →](https://osakakasugaayumu.github.io/threadx-core-g474re/)** · [按文件名查找](https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/index.html#site-file-lookup) · [查条件编译宏](https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/macro-index.html)

## 选一条路线

| 我想…… | 从这里开始 |
| --- | --- |
| **不接板，先读源码** | [裁剪版源码导读](https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/core/index.html)。从配置头和端口约定开始，读初始栈、PendSV 和就绪队列，再进入各个内核服务。 |
| **把 Demo 跑在 G474RE 上** | [Windows 编译与烧录教程](https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/core/build-flash.html)。先编译板级镜像、认板并备份；烧录后看线程和队列计数，不必等串口打印。 |

**离线阅读：** 克隆或解压整个仓库，双击根目录的 `index.html`。保留目录结构即可，不需要搭建网站。

## 第一次走读：发送一条消息，为什么要关中断？

1. **看处理器。** [两行汇编](threadx_core/tx_thread_interrupt_disable.S#L70-L71)先读取 `PRIMASK`，再用 `CPSID i` 关中断。[`tx_port.h`](threadx_core/tx_port.h#L315-L316) 用 `TX_DISABLE` 保存旧状态，用 `TX_RESTORE` 恢复它，而不是一律开中断。
2. **看队列。** [`tx_queue_send.c`](threadx_core/tx_queue_send.c#L99-L113) 在临界区里检查等待线程数和剩余空间。如果队列已满且发送线程可以等待，注意它[先恢复中断，再调用挂起函数](threadx_core/tx_queue_send.c#L327-L334)。
3. **回到 Demo。** [线程 1 发送](threadx_core/demo_threadx.c#L218)，[线程 2 接收](threadx_core/demo_threadx.c#L244)，操作的是同一个 `queue_0`。读完再对照[板上三组计数](https://osakakasugaayumu.github.io/threadx-core-g474re/reports/core-c5/index.html#samples)：收发计数都在增加，但同一次采样不要求两者相等。

## 两份源码怎么选？

- **日常走读用 [`threadx_core/`](threadx_core/)。** 只保留当前 [`autoconf.h`](threadx_core/autoconf.h) 配置下需要的分支，不会因为 Demo 没调用某个 API 就删除它。
- **对照宏分支用 [`main/`](main/)。** 这是已接入同一配置头、尚未裁剪的学习版，不是上游原件。要查原始路径和上游源码，打开[未裁剪目录](https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/index.html)。

| 阅读范围 | 每套文件数 | 裁剪前有效行 | 裁剪后有效行 |
| --- | ---: | ---: | ---: |
| 单核内核 + Cortex-M4/GNU 端口 | 208 | 16,878 | **10,819** |
| 全部阅读文件（另含 6 份 Demo／支撑、1 份项目配置） | 215 | 17,490 | **11,419** |

有效行不含空行和纯注释。目录里原 214 份文件的「81 已用 / 133 未直接用」是历史 Demo 的构建与链接结果，**不是**运行覆盖率，也不决定哪些文件被保留。

## 去哪里核对结果？

- **源码改了多少？** [C2 · 逐文件行数与 SHA-256](https://osakakasugaayumu.github.io/threadx-core-g474re/reports/core-c2/index.html) 可用来核对裁剪前后的行数和本地文件字节。
- **镜像有没有变？** [C3 · 编译与链接等价性](https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/core/verification.html) 分别比较通用目标和板级目标各自的裁前、裁后镜像。通用镜像不能拿来烧这块板。
- **八个线程真的在跑吗？** [C5 · 烧录、读回与运行摘要](https://osakakasugaayumu.github.io/threadx-core-g474re/reports/core-c5/index.html) 给出历史板级镜像的实测结果，不能把它当作教程中可移植镜像的上板记录。

仓库不分发固件、Flash 备份、逐条调试日志或本机编译数据库。本项目用于学习，不是车规认证交付物。有关上游代码及许可的适用范围，见[第三方来源说明](THIRD-PARTY-NOTICES.rst)和 [ThreadX MIT 许可原文](LICENSES/ThreadX-MIT.txt)。

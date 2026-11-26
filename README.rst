ThreadX 内核阅读与上板 · NUCLEO-G474RE
============================================================

`在线阅读：中文指南首页 <https://osakakasugaayumu.github.io/threadx-core-g474re/>`_ · `编译与烧录教程 <https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/core/build-flash.html>`_ · `直接读裁剪内核 <https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/core/index.html>`_ 。没有开发板也能走完源码路线。

从哪里开始？
------------

* **想先把板跑起来**：从教程核对工具和 ST 软件包，使用固定配置从源码编译 NUCLEO-G474RE 的镜像。认板、备份，再自行决定是否写 Flash。用线程和队列计数判断 Demo 是否在运行，而不是等待串口打印。
* **想先看内核**：从 `裁剪目录 <https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/core/index.html>`_ 的配置头开始，看临界区、初始栈、PendSV 和调度，再读内存与通信组件。需要比较其他编译分支时，切换到 `未裁剪目录 <https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/index.html>`_ 或 `宏索引 <https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/macro-index.html>`_ 。
* **离线读**：克隆或解压整个仓库，直接双击根目录的 ``index.html`` ，保留目录结构；不需要构建网站。GitHub 的文件视图显示 HTML 源码，网页版请用上面的在线阅读链接。

源码和证据
------------

``main/`` 是裁剪前的对照源码，``threadx_core/`` 是固定 ``autoconf.h`` 的裁剪版。两边各 215 份：208 份 ThreadX 6.2.0 单核及 Cortex-M4/GNU 端口、6 份 Demo 与构建/启动支撑、1 份项目配置。208 份内核/端口裁后为 **10,819 有效行**；计入支撑和配置共 **11,419 有效行**。只删除当前编译配置不选的分支，不按 Demo 是否调用删除 API。原 214 份的 81 已用 / 133 未直接用是历史构建标记，不是运行覆盖率。

* `C2 逐文件清单、行数与哈希 <https://osakakasugaayumu.github.io/threadx-core-g474re/reports/core-c2/index.html>`_ ：可核对两套学习源码的字节。
* `C3 编译链接结果 <https://osakakasugaayumu.github.io/threadx-core-g474re/demo_threadx/core/verification.html>`_ ：通用和板级目标各自与裁前基线比较。通用镜像不是该板的烧录镜像。
* `C5 真机验收摘要 <https://osakakasugaayumu.github.io/threadx-core-g474re/reports/core-c5/index.html>`_ ：既有板级镜像的读回、8 个线程和队列收发计数。本仓库不分发逐条 SWD 日志、Flash 备份、已编译固件或本机编译数据库。教程另列可移植镜像的哈希；两者不是同一次上板记录。

学习资料，不是车规认证交付物。源码版本、项目改动、上游许可和适用范围见 `第三方来源说明 <THIRD-PARTY-NOTICES.rst>`_ 与 `ThreadX MIT 许可 <LICENSES/ThreadX-MIT.txt>`_ 。

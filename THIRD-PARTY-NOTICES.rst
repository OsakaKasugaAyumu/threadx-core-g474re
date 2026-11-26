ThreadX 源码来源与许可
========================================

本说明只针对 ``main/`` 和 ``threadx_core/`` 中来自 ThreadX 的代码，包括派生文件保留的上游部分。
它不为本站原创的中文导读、页面样式或本地新增内容指定许可证，也不适用于整套 ST 软件包。

源码来自哪一版？
----------------------------------------

本读本使用 **ThreadX 6.2.0**。源码取自 `ST 的 X-CUBE-AZRTOS-G4 软件包 <https://github.com/STMicroelectronics/x-cube-azrtos-g4>`_
中的 ``Middlewares/ST/threadx/``。核对基线为 Eclipse ThreadX 官方仓库的 ``v6.2.0_rel``，
`固定提交 3e8e85cdc1e51f9de577457377a4f9a4f6b9b82f <https://github.com/eclipse-threadx/threadx/commit/3e8e85cdc1e51f9de577457377a4f9a4f6b9b82f>`_ 。

``main/`` 采用扁平目录；需要查询某个文件的原始路径，可查看 `逐文件目录 <demo_threadx/index.html>`_ 。以下是原 214 份。新增的第 215 份 ``main/autoconf.h`` 由本项目编写，不是 ThreadX 上游文件、ST 配置或经过认证的配置，不计入以下分类：

- **208 份内核和移植文件**：197 份来自单核 ``common/inc/`` 和 ``common/src/``，11 份来自 ``ports/cortex_m4/gnu/inc/`` 和 ``ports/cortex_m4/gnu/src/``。
- **4 份上游示例文件**：``samples/demo_threadx.c``，以及 ``ports/cortex_m4/gnu/example_build/`` 下的 ``cortexm4_vectors.S``、``cortexm4_crt0.S`` 和 ``tx_initialize_low_level.S``。合计 212 份的历史输入曾与固定提交逐一比对，字节相同；这不表示本仓库的学习副本仍是这些原件。唯有 ``cortexm4_crt0.S`` 在 ST 包和本仓库中保留小写扩展名 ``.s``。
- **2 份本例构建文件**：历史输入的配置头 ``tx_user.h`` 与上游 ``common/inc/tx_user_sample.h`` 字节相同；链接脚本 ``demo_threadx.crt0.ld`` 则基于 `上游 sample_threadx.ld <https://github.com/eclipse-threadx/threadx/blob/3e8e85cdc1e51f9de577457377a4f9a4f6b9b82f/ports/cortex_m4/gnu/example_build/sample_threadx.ld>`_ 为本例启动文件调整，不是上游原件。其文件头记录了原脚本的 SHA-256。

仓库里的源码是怎样得到的？
----------------------------------------

- 历史 214 份是来源和阅读行号基线。``main/`` 在这 214 份各自的原空行显式加入一条 ``#include "autoconf.h"`` ，因此不能称为上游字节镜像。第 215 份是项目配置，不属于历史 214 份。
- ``threadx_core/`` 以这份配置裁掉未选条件分支，保留了来源中的版权头。C2 单列编译现场仍需要的条件与定位行；并没有按 Demo 的调用覆盖删掉整组 API。`机器可读清单 <reports/core-c2/manifest.json>`_ 提供两棵树逐文件的 SHA-256。
- 历史固定行号参考仍可查旧版本；本仓库的两棵源码树、浏览样式和导读均可离线阅读。G474RE 教程的厂商启动文件、CMSIS 和链接脚本从用户另取的 ST 软件包读取，不把整套 SDK、工具链或固件放入本仓库。

为什么 6.2.0 的文件头仍写着旧许可？
-----------------------------------------------

`6.2.0 标签中的历史许可 <https://github.com/eclipse-threadx/threadx/blob/3e8e85cdc1e51f9de577457377a4f9a4f6b9b82f/LICENSE.txt>`_ 是当时的 Microsoft Azure RTOS 条款。本读本保留历史文件头，不改写版权声明。

- `微软的开源公告（2024-04-04） <https://techcommunity.microsoft.com/blog/iotblog/azure-rtos-transition-to-open-source-is-now-complete/4105027>`_ 明确说明，此前发布的 Azure RTOS 6.x 版本也可按 MIT 许可使用。
- `Eclipse ThreadX 项目负责人的公开答复（2025-07-30） <https://www.eclipse.org/lists/threadx/msg00135.html>`_ 确认这包括上游仓库的旧版本；没有回改旧标签，是为了保留提交历史。

随附的 `英文 MIT 许可原文 <LICENSES/ThreadX-MIT.txt>`_ 逐字节取自 `官方 v6.4.1_rel 的 LICENSE.txt <https://github.com/eclipse-threadx/threadx/blob/07eac307405dbf09d409844e31c37b0649d6c074/LICENSE.txt>`_ 。6.4.1 仅是这份许可文本的出处，不是本读本的源码版本。
依照 MIT 条款，再分发该软件的副本或其实质性部分时，须同时保留版权声明和许可声明。

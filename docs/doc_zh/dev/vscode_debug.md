---
title: 使用 VSCode 在线调试
---

## 调试

这里主要提供 vscode + gdb 的调试方法，看不太懂可以先跳过，可以先使用代码加`printf`打印的方式调试。

**(1).** 对于在本机（PC）运行，VSCode + GDB 在线调试

这里以 PC 为 Linux 系统为例：

* 添加 MaixCDK（第一次试用推荐这样） 或者 工程目录到 VSCode 工作区
* 拷贝 [tools/vscode/vscode_local_debug/.vscode](../../../tools/vscode/vscode_local_debug/.vscode) 目录到上一步的工作目录下
* 根据`.vscode`是在 MaixCDK 还是在工程目录下，修改`.vscode/launch.json`中的`cwd`字段
* 按键盘 `F5` 即可开始调试
> windows 也类似，修改`.vscode`里面的相关命令和路径即可

**(2).** 对于在嵌入式设备（/远程设备，带 Linux 系统）调试

使用 VSCode + gdbserver 在嵌入式设备（/远程设备，带 Linux 系统）调试

这里以 PC 为 Linux 系统为例：

* 先保证远程设备有`gdbserver`这个程序，以及 PC 有`gdb-multiarch`这个程序
* 将 [tools/vscode/vscode_remote_debug/.vscode](../../../tools/vscode/vscode_remote_debug/.vscode) 目录拷贝到工程目录下
* 编辑 `launch.json` 和 `build_run_gdbserver.sh` 文件，修改里面的路径和命令，以及用户名等。
> 建议先将 PC 的 ssh key 加入到远程设备的 `~/.ssh/authorized_keys` 文件中，这样就不需要输入密码了。
* 每次调试需要执行 `build_run_gdbserver.sh` 脚本，然后在 VSCode 中按 `F5` 即可开始调试
> 脚本会编译工程，然后拷贝可执行文件到远程设备，并且启动 `gdbserver`。
> 按 F5 启动调试时， VSCode 使用 GDB 连接到远程设备的`gdbserver`以调试。
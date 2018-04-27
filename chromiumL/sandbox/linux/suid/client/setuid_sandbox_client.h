#ifndef SANDBOX_LINUX_SUID_CLIENT_SETUID_SANDBOX_CLIENT_H_
#define SANDBOX_LINUX_SUID_CLIENT_SETUID_SANDBOX_CLIENT_H_

#include<memory>

#include"sandbox/sandbox_export.h"

namespace sanbox {


// 使用setuid sandbox的辅助类。在被setuid helper（sandbox.cc）执行后使用.
// 进程A启动沙箱化的进程B的典型用法：
// 
// 5,B使用CloseDummyFile()关闭空文件描述符
// 6,B执行初始化，请求文件系统
// 6, (可选) B使用sandbox::Credentials::HasOpenDirectory()验证没有打开的目录
// 7,B准备程序init进程。B不能接受任何其他进程的消息，除了SIGKILL。
// 
// 8,B调用ChrootMe()请求被chroot，通过状态函数请求其他沙箱状态
class SANDBOX_EXPORT SetuidSandboxClient {

};

}

#endif
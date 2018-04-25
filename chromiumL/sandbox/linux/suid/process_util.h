#ifndef SANDBOX_LINUX_SUID_PROCESS_UTIL_H_
#define SANDBOX_LINUX_SUID_PROCESS_UTIL_H_

#include<sys/types.h>

// 修改/proc/process/oom_score_adj文件，使Linux OOM机制优先杀死某些进程
// 范围[-1000, 1000]，用户可访问范围[0, 1000]
// 
// 旧Linux系统，设置oom_adj，范围是[0, 15]
bool AdjustOOMScore(pid_t process, int score);

#endif

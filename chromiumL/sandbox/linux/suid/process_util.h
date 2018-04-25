#ifndef SANDBOX_LINUX_SUID_PROCESS_UTIL_H_
#define SANDBOX_LINUX_SUID_PROCESS_UTIL_H_

#include<sys/types.h>

// �޸�/proc/process/oom_score_adj�ļ���ʹLinux OOM��������ɱ��ĳЩ����
// ��Χ[-1000, 1000]���û��ɷ��ʷ�Χ[0, 1000]
// 
// ��Linuxϵͳ������oom_adj����Χ��[0, 15]
bool AdjustOOMScore(pid_t process, int score);

#endif

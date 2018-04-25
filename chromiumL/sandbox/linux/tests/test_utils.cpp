#include"sandbox/tests/test_utils.h"

#include<sys/wait.h>

#include"base/posix/eintr_wrapper.h"
#include"base/logging.h"

/*
int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
等待一个子进程改变状态
挂起当前线程，直到该线程所在进程的某个子进程改变状态
如果在调用waitid之前，一个子进程改变了状态，waitid立即返回
infop返回一个子进程的当前状态;idtype和id指定等待的子进程；
如果指定NOHANG并且没有子进程等待，返回0；如果由于子进程状态改变返回，返回值为0；否则-1
*/
bool TestUtils::CurrentProcessHasChildren()
{
	siginfo_t process_info;
	int ret = HANDLE_EINTR(waitid(P_ALL, 0, &process_info, WEXITED | WNOHANG | WNOWAIT));
	if (-1 == ret)
	{
		PCHECK(ECHILD == errno); // ECHILD调用进程没有正在退出的没有wait的子进程
		return false; // ??没有子进程
	}
	else // ??有子进程
		return true;
}

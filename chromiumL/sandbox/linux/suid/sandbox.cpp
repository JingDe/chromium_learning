#include "sandbox/linux/suid/common/sandbox.h"

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<climits>

#include<unistd.h>
#include<errno.h>
#include<sys/socket.h>

#include"process_util.h"

#if !defined(CLONE_NEWPID)
#define CLONE_NEWPID 0x20000000
#endif

#if !defined(CLONE_NEWNET)
#define CLONE_NEWNET 0x40000000
#endif

using namespace sandbox;

static void FatalError(const char* msg, ...)
__attribute__((noreturn, format(printf, 1, 2)));
// 使编译器检查函数声明和函数实际调用参数之间的格式化字符串是否匹配



bool CheckAndExportApiVersion()
{
	int api_number = -1;
	char* api_string = getenv(kSandboxEnvironmentApiRequest);
	if (!api_string)
		api_number = 0;
	else
	{
		errno = 0;
		char* endptr = NULL;
		long long_api_number = strtol(api_string, &endptr, 10);
		if (!endptr || *endptr || errno != 0 || long_api_number<INT_MIN || long_api_number>INT_MAX)
			return false;
		api_number = long_api_number;
	}

	if (api_number != kSUIDSandboxApiNumber) // 1
	{
		fprintf(stderr, "The setuid sandbox provides API version %d, "
			"but you need %d\n"
			"Please read "
			"https://chromium.googlesource.com/chromium/src/+/master/docs/linux_suid_sandbox_development.md."
			"\n\n",
			kSUIDSandboxApiNumber,
			api_number);
	}

	char version_string[64];
	snprintf(version_string, sizeof(version_string), "%d", kSUIDSandboxApiNumber);
	if (setenv(kSandboxEnvironmentApiProvides, version_string, 1))
	{
		perror("setenv");
		return false;
	}
	return true;
}

static bool MoveToNewNamespaces()
{
	const int kCloneExtraFlags[] = { CLONE_NEWPID | CLONE_NEWNET, CLONE_NEWPID, };

	int sync_fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sync_fds))
		// 创建一对连接的套接字
	{
		FatalError("Failed to create a socketpair");
	}
}

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		if (argc <= 0)
			return 1;
		fprintf(stderr, "Usage: %s <renderer process> <args...>\n", argv[0]);
		return 1;
	}

	// sandbox --get-api
	if (argc == 2 && 0 == strcmp(argv[1], kSuidSandboxGetApiSwitch)) // --get-api
	{
		printf("%d\n", kSUIDSandboxApiNumber); // 1
	}

	// sandbox --adjust-oom-score pid_t score
	// 不能为沙箱化的渲染进程修改/proc/pid/oom_adj，因为这些文件由root拥有
	// WHY?
	if (argc == 4 && (0 == strcmp(argv[1], kAdjustOOMScoreSwitch))) // --adjust-oom-score
	{
		char* endptr = NULL;
		long score;
		errno = 0;
		unsigned long pid_ul = strtoul(argv[2], &endptr, 10); // string转无符号长整数
		if (pid_ul == ULONG_MAX || !endptr || *endptr || errno != 0)
			return 1;
		pid_t pid = pid_ul;

		endptr = NULL;
		errno = 0;
		score = strtol(argv[3], &endptr, 10);
		if (score == LONG_MAX || score == LONG_MIN || !endptr || *endptr || errno != 0)
		{
			return 1;
		}
		return AdjustOOMScore(pid, score); // 修改/proc/pid/oom_adj内容score
	}

	if (!CheckAndExportApiVersion())
		return 1;

	if (geteuid() != 0)
	{
		fprintf(stderr, "The setuid sandbox is not running as root. Common causes:\n"
            "  * An unprivileged process using ptrace on it, like a debugger.\n"
			"  * A parent process set prctl(PR_SET_NO_NEW_PRIVS, ...)\n");
	}

	if (!MoveToNewNamespaces())
		return 1;
}
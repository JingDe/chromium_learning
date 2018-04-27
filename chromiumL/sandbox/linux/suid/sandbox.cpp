#include "sandbox/linux/suid/common/sandbox.h"

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<climits>
#include<cstdarg>

#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<sys/socket.h>
#include<sys/syscall.h>
#include<sys/prctl.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/resource.h>
#include<sched.h>

#include"sandbox/linux/suid/process_util.h"
#include"sandbox/linux/suid/common/suid_unsafe_environment_variables.h"
#include"base/posix/eintr_wrapper.h"

#if !defined(CLONE_NEWPID)
#define CLONE_NEWPID 0x20000000
#endif

#if !defined(CLONE_NEWNET)
#define CLONE_NEWNET 0x40000000
#endif

using namespace sandbox;

	// 使编译器检查函数声明和函数实际调用参数之间的格式化字符串是否匹配
static void FatalError(const char* msg, ...) __attribute__((noreturn, format(printf, 1, 2)));
static bool DropRoot();
static void WaitForChildAndExit(pid_t child_pid);
static void ExitWithErrorSignalHandler(int signal);
bool CheckAndExportApiVersion();
static bool MoveToNewNamespaces();
static bool SpawnChrootHelper();
static bool SetupChildEnvironment();

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
		return 0;
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
	if (!SpawnChrootHelper())
		return 1;
	if (!DropRoot())
		return 1;
	if (!SetupChildEnvironment())
		return 1;

	execv(argv[1], &argv[1]);
	FatalError("execv failed");

	return 1;
}

static bool DropRoot()
{
	if (prctl(PR_SET_DUMPABLE, 0, 0, 0, 0)) // 进程控制，prctl(PR_SET_DUMPABLE, SUID_DUMP_DISABLE, 0, 0, 0)
	{
		perror("prctl(PR_SET_DUMPABLE)");
		return false;
	}

	if (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0))
	{
		perror("Stilll dumpable after prctl(PR_SET_DUMPABLE)");
		return false;
	}

	gid_t rgid, egid, sgid;
	if (getresgid(&rgid, &egid, &sgid)) // 返回调用进程的实际用户id、有效用户id、saved用户id
	{
		perror("getresgit");
		return false;
	}

	if (setresgid(rgid, rgid, rgid))
	{
		perror("setresgid");
		return false;
	}

	uid_t ruid, euid, suid;
	if (getresuid(&ruid, &euid, &suid))
	{
		perror("getresuid");
		return false;
	}

	if (setresuid(ruid, ruid, ruid))
	{
		perror("setresuid");
		return false;
	}

	return true;
}

// 阻塞直到child_pid退出，然后退出，保存退出码
static void WaitForChildAndExit(pid_t child_pid)
{
	int exit_code = -1;
	siginfo_t reaped_child_info;

	if (signal(SIGABRT, ExitWithErrorSignalHandler) == SIG_ERR)
		FatalError("Failed to change signal handler");

	int wait_ret = HANDLE_EINTR(waitid(P_PID, child_pid, &reaped_child_info, WEXITED));

	if (!wait_ret  &&  reaped_child_info.si_pid == child_pid)
	{
		if (reaped_child_info.si_code == CLD_EXITED)
			exit_code = reaped_child_info.si_status;
		else
			exit_code = 0;
	}
	_exit(exit_code);
}

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

	for (size_t i = 0; i < sizeof(kCloneExtraFlags) / sizeof(kCloneExtraFlags[0]); i++)
	{
		// 克隆子进程
		// int clone(int (*fn)(void*), void *child_stack, int flags, void *arg, ...);
		pid_t pid = syscall(__NR_clone, SIGCHLD | kCloneExtraFlags[i], 0, 0, 0);
		const int clone_errno = errno;

		if (pid > 0) // 父进程
		{
			if (!DropRoot())
				FatalError("Could not drop privileges");
			else
			{
				// 关闭sync_fds[0]和sync_fds[1]的读端
				if (close(sync_fds[0]) || shutdown(sync_fds[1], SHUT_RD))
					FatalError("Could not close socketpair");
				// 在zygote运行之前，父进程关闭zygote
				if (close(kZygoteIdFd))
					FatalError("close");

				// 通知子进程继续
				if (HANDLE_EINTR(send(sync_fds[1], "C", 1, MSG_NOSIGNAL)) != 1)
					FatalError("send");
				if (close(sync_fds[1]))
					FatalError("close");

				// 关闭子进程，不希望子进程成为外部PID空间的init的子进程
				WaitForChildAndExit(pid);
			}

			FatalError("Not reached");
		}

		if (pid == 0) // 子进程
		{
			if (close(sync_fds[1]) || shutdown(sync_fds[0], SHUT_WR))
				FatalError("Could not close socketpair");

			// 等待父进程确认已关闭kZygoteIdFd再继续
			char should_continue;
			if (HANDLE_EINTR(read(sync_fds[0], &should_continue, 1)) != 1)
				FatalError("Read on socketpair");
			if (close(sync_fds[0]))
				FatalError("close");

			if (kCloneExtraFlags[i] & CLONE_NEWPID)
			{
				setenv(kSandboxPIDNSEnvironmentVarName, "", 1);
			}
			else
				unsetenv(kSandboxPIDNSEnvironmentVarName);

			if (kCloneExtraFlags[i] & CLONE_NEWNET)
				setenv(kSandboxNETNSEnvironmentVarName, "", 1);
			else
				unsetenv(kSandboxNETNSEnvironmentVarName);

			break;
		}

		// 子进程
		// 若返回错误EINVAL，表明系统不支持该信号，继续下一组
		// 若返回错误不是EINVAL，
		if (errno != EINVAL)
		{
			fprintf(stderr, "Failed to move to new namespace:");
			if (kCloneExtraFlags[i] & CLONE_NEWPID) {
				fprintf(stderr, " PID namespaces supported,");
			}
			if (kCloneExtraFlags[i] & CLONE_NEWNET) {
				fprintf(stderr, " Network namespace supported,");
			}
			fprintf(stderr, " but failed: errno = %s\n", strerror(clone_errno));
			return false;
		}
	}

	return true;
}

static void ExitWithErrorSignalHandler(int signal)
{
	const char msg[] = "\nThe setuid sandbox got signaled, exiting.\n";
	if (-1 == write(2, msg, sizeof(msg) - 1))
	{

	}
	_exit(1);
}


// chroot到helper的/proc/self目录，如果wait helper，该目录为空，即使helper是僵尸进程
#define SAFE_DIR "/proc/self/fdinfo"
#define SAFE_DIR2 "/proc/self/fd"

static bool SpawnChrootHelper()
{
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 1)
	{
		perror("socketpair");
		return false;
	}

	char* safedir = NULL;
	struct stat sdir_stat;
	if (!stat(SAFE_DIR, &sdir_stat) && S_ISDIR(sdir_stat.st_mode))
	{
		safedir = SAFE_DIR;
	}
	else if (!stat(SAFE_DIR2, &sdir_stat) && S_ISDIR(sdir_stat.st_mode))
		safedir = SAFE_DIR2;
	else
	{
		fprintf(stderr, "Could not find %s\n", SAFE_DIR2);
		return false;
	}

	const pid_t pid = syscall(__NR_clone, CLONE_FS | SIGCHLD, 0, 0, 0);

	if (pid == -1)
	{
		perror("clone");
		close(sv[0]);
		close(sv[1]);
		return false;
	}

	if (pid == 0) // 子进程
	{
		const struct rlimit nofile = { 0, 0 };
		if (setrlimit(RLIMIT_NOFILE, &nofile))
			FatalError("Setting RLIMIT_NOFILE");

		if (close(sv[1]))
			FatalError("close");

		char msg;
		ssize_t bytes;
		do {
			bytes = read(sv[0], &msg, 1);
		} while (bytes == -1 && errno == EINTR);

		if (bytes == 0)
			_exit(0);
		if (bytes != 1)
			FatalError("read");

		if (msg != kMsgChrootMe)
			FatalError("Unknown message from sandboxed process");

		if(chdir(safedir))
			FatalError("Cannot chdir into /proc/ directory");

		if (chroot(safedir))
			FatalError("Cannot chroot into /proc/ directory");

		if (chdir("/"))
			FatalError("Cannot chdir to / after chroot");

		const char replay = kMsgChrootSuccessful;
		do {
			bytes = write(sv[0], &replay, 1);
		} while (bytes == -1 && errno == EINTR);

		if (bytes != 1)
			FatalError("Writing replay");

		_exit(0);
		// 成为僵尸进程，/proc/self/fd(info)成为空目录，并chroot到此目录
		// 如果父进程wait次进程，根目录将消失
	}

	if (close(sv[0]))
	{
		close(sv[1]);
		perror("close");
		return false;
	}

	char desc_str[64];
	int printed = snprintf(desc_str, sizeof(desc_str), "%u", sv[1]);
	if (printed < 0 || printed >= (int)sizeof(desc_str))
	{
		fprintf(stderr, "Filed to snprintf\n");
		return false;
	}

	if (setenv(kSandboxDescriptorEnvironmentVarName, desc_str, 1))
	{
		perror("setenv");
		close(sv[1]);
		return false;
	}

	char helper_pid_str[64];
	printed = snprintf(helper_pid_str, sizeof(helper_pid_str), "%u", pid);
	if (printed < 0 || printed >= (int)sizeof(helper_pid_str)) {
		fprintf(stderr, "Failed to snprintf\n");
		return false;
	}

	if (setenv(kSandboxHelperPidEnvironmentVarName, helper_pid_str, 1)) {
		perror("setenv");
		close(sv[1]);
		return false;
	}

	return true;
}

static bool SetupChildEnvironment()
{
	unsigned i;

	// 由于是SUID，ld.so可能清楚一些环境变量
	// 子进程可能需要这些环境变量
	// 此处已抛弃root权限，所以只能在在用户允许下运行一个二进制

	for (i = 0; kSUIDUnsafeEnvironmentVariables[i]; ++i)
	{
		const char* const envvar = kSUIDUnsafeEnvironmentVariables[i];
		char* const saved_envvar = SandboxSavedEnvironmentVariable(envvar);
		if (!saved_envvar)
			return false;

		const char* const value = getenv(saved_envvar);
		if (value) {
			setenv(envvar, value, 1 /* overwrite */);
			unsetenv(saved_envvar);
		}

		free(saved_envvar);
	}

	return true;
}

static void FatalError(const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);

	vfprintf(stderr, msg, ap);
	fprintf(stderr, ": %s\n", strerror(errno));
	fflush(stderr);
	va_end(ap);
	_exit(1);
}
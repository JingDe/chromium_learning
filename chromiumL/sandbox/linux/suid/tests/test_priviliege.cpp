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
#include<sys/types.h>
#include<sched.h>

#define HANDLE_EINTR(x) ({ \
	int eintr_wrapper_counter=0; \
	decltype(x) eintr_wrapper_result; \
	do { \
		eintr_wrapper_result = (x); \
	} while (eintr_wrapper_result == -1 && errno == EINTR && eintr_wrapper_counter++ < 100); \
	eintr_wrapper_result; \
})

void  printtid(int no)
{
	int tid= static_cast<pid_t>(syscall(SYS_gettid));
	fprintf(stdout, "%d: pid = %d, tid=%d\n", no, getpid(), tid);
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

bool DropRoot()
{
	gid_t rgid, egid, sgid;
	getresgid(&rgid, &egid, &sgid);
	printf("%d, %d, %d\n", rgid, egid, sgid);
		
	uid_t ruid, euid, suid;
	getresuid(&ruid, &euid, &suid);
	printf("%d, %d, %d\n", ruid, euid, suid);

	printf("after\n");
	setresgid(rgid, rgid, rgid);
	printf("%d, %d, %d\n", rgid, egid, sgid);

	setresuid(ruid, ruid, ruid);
	printf("%d, %d, %d\n", ruid, euid, suid);

	return true;
}

bool printID()
{
	gid_t rgid, egid, sgid;
	getresgid(&rgid, &egid, &sgid);
	printf("%d, %d, %d\n", rgid, egid, sgid);

	uid_t ruid, euid, suid;
	getresuid(&ruid, &euid, &suid);
	printf("%d, %d, %d\n", ruid, euid, suid);

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

// ����ֱ��child_pid�˳���Ȼ���˳��������˳���
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

static bool MoveToNewNamespaces()
{
	const int kCloneExtraFlags[] = { CLONE_NEWPID | CLONE_NEWNET, CLONE_NEWPID, };

	int sync_fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sync_fds))
		// ����һ�����ӵ��׽���
	{
		FatalError("Failed to create a socketpair");
	}

	for (size_t i = 0; i < sizeof(kCloneExtraFlags) / sizeof(kCloneExtraFlags[0]); i++)
	{
		// ��¡�ӽ���
		// int clone(int (*fn)(void*), void *child_stack, int flags, void *arg, ...);
		pid_t pid = syscall(SYS_clone, SIGCHLD | kCloneExtraFlags[i], 0, 0, 0); // SYS_clone 
		const int clone_errno = errno;

		if (pid > 0) // ������
		{
			printtid(2);

			if (!DropRoot())
				FatalError("Could not drop privileges");
			else
			{
				// �ر�sync_fds[0]��sync_fds[1]�Ķ���
				if (close(sync_fds[0]) || shutdown(sync_fds[1], SHUT_RD))
					FatalError("Could not close socketpair");
				

				// ֪ͨ�ӽ��̼���
				if (HANDLE_EINTR(send(sync_fds[1], "C", 1, MSG_NOSIGNAL)) != 1)
					FatalError("send");
				if (close(sync_fds[1]))
					FatalError("close");

				// �ر��ӽ��̣���ϣ���ӽ��̳�Ϊ�ⲿPID�ռ��init���ӽ���
				WaitForChildAndExit(pid);
				printf("hello\n");
			}

			FatalError("Not reached");
		}

		if (pid == 0) // �ӽ���
		{
			printtid(3);

			if (close(sync_fds[1]) || shutdown(sync_fds[0], SHUT_WR))
				FatalError("Could not close socketpair");

			// �ȴ�������ȷ���ѹر�kZygoteIdFd�ټ���
			char should_continue;
			if (HANDLE_EINTR(read(sync_fds[0], &should_continue, 1)) != 1)
				FatalError("Read on socketpair");
			if (close(sync_fds[0]))
				FatalError("close");

			

			break;
		}

		printtid(5);
		// �ӽ���
		// �����ش���EINVAL������ϵͳ��֧�ָ��źţ�������һ��
		// �����ش�����EINVAL��
		if (errno != EINVAL)
		{
			printtid(4);

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


int main()
{
	printtid(1);

	MoveToNewNamespaces();
	printf("after MoveToNewNamespaces\n");
	printtid(1);

	return 0;
}
#ifndef SANDBOX_LINUX_SUID_SANDBOX_H_
#define SANDBOX_LINUX_SUID_SANDBOX_H_

#if defined(__cplusplus)
namespace sandbox {
#endif
	
	// These are command line switches that may be used by other programs
	// (e.g. Chrome) to construct a command line for the sandbox.
	static const char kSuidSandboxGetApiSwitch[] = "--get-api";
	static const char kAdjustOOMScoreSwitch[] = "--adjust-oom-score";

	static const char kSandboxDescriptorEnvironmentVarName[] = "SBX_D"; // ����Ϊ�׽���fd
	static const char kSandboxHelperPidEnvironmentVarName[] = "SBX_HELPER_PID"; // ����Ϊhelper��pid

	static const int kSUIDSandboxApiNumber = 1; // SUIDɳ���API�汾
	static const char kSandboxEnvironmentApiRequest[] = "SBX_CHROME_API_RQ"; // ���������������API�汾
	static const char kSandboxEnvironmentApiProvides[] = "SBX_CHROME_API_PRV"; // ��������������ΪSUIDɳ���API�汾

	// This number must be kept in sync with common/zygote_commands_linux.h
	static const int kZygoteIdFd = 7;

	// These are the magic byte values which the sandboxed process uses to request
	// that it be chrooted.
	static const char kMsgChrootMe = 'C';
	static const char kMsgChrootSuccessful = 'O';

	// These are set if we have respectively switched to a new PID or NET namespace
	// by going through the setuid binary helper.
	static const char kSandboxPIDNSEnvironmentVarName[] = "SBX_PID_NS";
	static const char kSandboxNETNSEnvironmentVarName[] = "SBX_NET_NS";

#if defined(__cplusplus)
}
#endif

#endif

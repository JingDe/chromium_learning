#ifndef SANDBOX_LINUX_SUID_CLIENT_SETUID_SANDBOX_HOST_H_
#define SANDBOX_LINUX_SUID_CLIENT_SETUID_SANDBOX_HOST_H_

#include"sandbox/sandbox_export.h"
#include "base/files/file_path.h"
#include "base/files/scoped_file.h"
#include "base/process/launch.h"

namespace sandbox {

// ʹ��setuid sandbox�ĸ����ࡣ�ڱ�setuid helper��sandbox.cc��ִ�к�ʹ��.
// ����A����ɳ�仯�Ľ���B�ĵ����÷���
// 1,A����SetupLaunchEnvironment()
// 2,A����base::CommandLine��ʹ��PrepenWrapper()�޸ģ���������GetSandboxBinaryPath()�޸ģ�
// 3��Aʹ��SetupLauchOptions()��Ϊsetuidɳ��ABI�ṩһ����������
// 4��Aʹ��base::LauchProcess����B��ʹ�������base::CommandLine
// 
class SANDBOX_EXPORT SetuidSandboxHost {
public:
	static SetuidSandboxHost* Create();
	~SetuidSandboxHost();

	bool IsDisabledViaEnvironment();

	base::FilePath GetSandboxBinaryPath();

	void PrependWrapper(base::CommandLine* cmd_line);

	// Set-up the launch options for launching via the setuid sandbox.  Caller is
	// responsible for keeping |dummy_fd| alive until LaunchProcess() completes.
	// |options| must not be NULL. This function will append to
	// options->fds_to_remap so the caller should take care to append rather than
	// overwrite if it has additional FDs.
	// (Keeping |dummy_fd| alive is an unfortunate historical artifact of the
	// chrome-sandbox ABI.)
	void SetupLaunchOptions(base::LaunchOptions* options, base::ScopedFD* dummy_fd);

	// Set-up the environment. This should be done prior to launching the setuid
	// helper.
	void SetupLaunchEnvironment();

private:
	explicit SetuidSandboxHost(std::unique_ptr<base::Environment> env);

	// Holds the environment. Will never be NULL.
	std::unique_ptr<base::Environment> env_;

	DISALLOW_COPY_AND_ASSIGN(SetuidSandboxHost);
};


}

#endif
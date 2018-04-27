#ifndef SANDBOX_LINUX_SUID_CLIENT_SETUID_SANDBOX_HOST_H_
#define SANDBOX_LINUX_SUID_CLIENT_SETUID_SANDBOX_HOST_H_

#include"sandbox/sandbox_export.h"
#include "base/files/file_path.h"
#include "base/files/scoped_file.h"
#include "base/process/launch.h"

namespace sandbox {

// 使用setuid sandbox的辅助类。在被setuid helper（sandbox.cc）执行后使用.
// 进程A启动沙箱化的进程B的典型用法：
// 1,A调用SetupLaunchEnvironment()
// 2,A设置base::CommandLine，使用PrepenWrapper()修改（或者依赖GetSandboxBinaryPath()修改）
// 3，A使用SetupLauchOptions()，为setuid沙箱ABI提供一个空描述符
// 4，A使用base::LauchProcess启动B，使用上面的base::CommandLine
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
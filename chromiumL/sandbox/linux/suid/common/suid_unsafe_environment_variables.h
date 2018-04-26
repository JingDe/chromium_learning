#ifndef SANDBOX_LINUX_SUID_COMMON_SUID_UNSAFE_ENVIRONMENT_VARIABLES_H_
#define SANDBOX_LINUX_SUID_COMMON_SUID_UNSAFE_ENVIRONMENT_VARIABLES_H_

#include <stddef.h>

static const char* const kSUIDUnsafeEnvironmentVariables[] = {
	"LD_AOUT_LIBRARY_PATH",
	"LD_AOUT_PRELOAD",
	"GCONV_PATH",
	"GETCONF_DIR",
	"HOSTALIASES",
	"LD_AUDIT",
	"LD_DEBUG",
	"LD_DEBUG_OUTPUT",
	"LD_DYNAMIC_WEAK",
	"LD_LIBRARY_PATH",
	"LD_ORIGIN_PATH",
	"LD_PRELOAD",
	"LD_PROFILE",
	"LD_SHOW_AUXV",
	"LD_USE_LOAD_BIAS",
	"LOCALDOMAIN",
	"LOCPATH",
	"MALLOC_TRACE",
	"NIS_PATH",
	"NLSPATH",
	"RESOLV_HOST_CONF",
	"RES_OPTIONS",
	"TMPDIR",
	"TZDIR",
	NULL,
};

// Return a malloc allocated string containing the 'saved' environment variable
// name for a given environment variable.
static inline char* SandboxSavedEnvironmentVariable(const char* envvar) {
	const size_t envvar_len = strlen(envvar);
	const size_t kMaxSizeT = (size_t)-1;

	if (envvar_len > kMaxSizeT - 1 - 8)
		return NULL;

	const size_t saved_envvarlen = envvar_len + 1 /* NUL terminator */ +
		8 /* strlen("SANDBOX_") */;
	char* const saved_envvar = (char*)malloc(saved_envvarlen);
	if (!saved_envvar)
		return NULL;

	memcpy(saved_envvar, "SANDBOX_", 8);
	memcpy(saved_envvar + 8, envvar, envvar_len);
	saved_envvar[8 + envvar_len] = 0;

	return saved_envvar;
}

#endif

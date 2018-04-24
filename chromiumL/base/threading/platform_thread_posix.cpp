#include"base/threading/platform_thread.h"

#include<sys/types.h>

namespace base {
	PlatformThreadId PlatformThread::CurrentId()
	{
#if defined(OS_POSIX)  &&  defined(OS_AIX)
		return pthread_self();
#elif defined(OS_POSIX)  && !defined(OS_AIX)
		return static_cast<int64_t>(pthread_self()); // reinterpret_castÎÞÐ§£¿
#endif
	}

}
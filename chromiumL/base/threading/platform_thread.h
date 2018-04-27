#ifndef BASE_THREADING_PLATFORM_THREAD_H_
#define BASE_THREADING_PLATFORM_THREAD_H_

#include"base/base_export.h"
#include"base/macros.h"

#if defined(OS_WIN)
#include "base/win/windows_types.h"
#elif defined(OS_POSIX)
#include<pthread.h>
#endif

namespace base{

#if defined(OS_WIN)
	typedef DWORD PlatformThreadId;
#elif defined(OS_POSIX)
	typedef pid_t PlatformThreadId;
#endif

	// 底层线程函数的命名空间
	class BASE_EXPORT PlatformThread {
	public:
		class BASE_EXPORT Delegate {
		public:
			virtual void ThreadMain() = 0;

		protected:

		};

		static PlatformThreadId CurrentId();

	private:
		DISALLOW_IMPLICIT_CONSTRUCTORS(PlatformThread);
	};

}

#endif
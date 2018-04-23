#ifndef BASE_POSIX_ENINTR_WRAPPTER_H_
#define BASE_POSIX_ENINTR_WRAPPTER_H_

#if defined(OS_POSIX)  &&  !defined(OS_FUCHSIA)

#include<errno.h>

#if defined(NDEBUG)
#define HANDLE_EINTR(x) ({ \
	decltype(x) eintr_wrapper_result; \
	do { \
		eintr_wrapper_result=(x); \
	}while(eintr_wrapper_result ==-1  &&  errno==EINTR); \
	eintr_wrapper_result; \
})
#else // DEBUG模式下，检查100次x，直到x不等于-1或发生EINTR之外的错误
#define HANDLE_EINTR(x) ({ \
	int eintr_wrapper_counter=0; \
	decltype(x) eintr_wrapper_result; \
	do { \
		eintr_wrapper_result = (x); \
	} while (eintr_wrapper_result == -1 && errno == EINTR && eintr_wrapper_counter++ < 100); \
	eintr_wrapper_result; \
})
#endif

#else
#define HANDLE_EINTR(x) (x)
#endif

#endif

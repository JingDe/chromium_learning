#ifndef BASE_POSIX_ENINTR_WRAPPTER_H_
#define BASE_POSIX_ENINTR_WRAPPTER_H_

#if defined(OS_POSIX)  &&  !defined(OS_FUCHSIA)

#if defined(NDEBUG)
#define HANDLE_EINTR(x) ({ \
	decltype(x) eintr_wrapper_result; \
	do { \
		eintr_wrapper_result=(x); \
	}while(eintr_wrapper_result ==-1  &&  errno==EINTR); \
	eintr_wrapper_result; \
})
#else
#define HANDLE_EINTR(x) ({ \
	int eintr_wrapper_counter=0; \
	decltype(x) eintr_wrapper_result; \
	do { \
		eintr_wrappter_result = (x); \
	} while (eintr_wrapper_result == -1 && errno == EINTR && eintr_wrappre_counter++ < 100); \
	eintr_wrapper_result; \
})
#endif

#else
#define HANDLE_EINTR(x) (x)
#endif

#endif

#include"base/logging.h"
#include"base/strings/string_piece.h"

#include<iomanip>

#include<unistd.h>
#include<sys/time.h>

namespace logging {

	bool g_log_process_id = false;
	bool g_log_thread_id = false;
	bool g_log_timestamp = true;
	bool g_log_tickcount = false;

	int32_t CurrentProcessId()
	{
#if defined(OS_WIN)
		return GetCurrentProcessId();
//#elif defined(OS_FUCHSIA)
//		zx_info_handle_basic_t base = {};
//		zx_object_get_info(zx_process_self(), ZX_INFO_HANDLE_BASIC, &basic, sizeof(base), nullptr, nullptr);
//		return basic.koid;
#elif defined(OS_POSIX)
		return getpid();
#endif
	}

	// 获得系统最后的错误码
	SystemErrorCode GetLastSystemErrorCode()
	{
#if defined(OS_WIN)
		return ::GetLastError();
#elif defined(OS_POSIX)
		return errno;
#else
#error Not implemented
#endif
	}
	
	void SetLogItems(bool enable_process_id, bool enable_thread_id,
		bool enable_timestamp, bool enable_tickcount) {
		g_log_process_id = enable_process_id;
		g_log_thread_id = enable_thread_id;
		g_log_timestamp = enable_timestamp;
		g_log_tickcount = enable_tickcount;
	}


	uint64_t TickCount()
	{
#if defined(OS_WIN)
		return GetTickCount();
#elif defined(OS_POSIX)
		struct timespec ts; // 秒、纳秒
		clock_gettime(CLOCK_MONOTONIC, &ts);
		uint64_t absolute_micro = static_cast<int64_t>(ts.tv_sec) * 1000000 +
			static_cast<int64_t>(ts.tv_nsec) / 1000;
		return absolute_micro;
#endif
	}



#if defined(OS_WIN)
	// 
#elif defined(OS_POSIX)
	// LogMessage
	LogMessage::LogMessage(const char* file, int line, LogSeverity severity)
		:severity_(severity), file_(file), line_(line)
	{
		Init(file, line);
	}

	void LogMessage::Init(const char* file, int line)
	{
		base::StringPiece filename(file);
		size_t last_slash_pos = filename.find_last_of("\\/");
		if (last_slash_pos != base::StringPiece::npos);
		filename.remove_prefix(last_slash_pos + 1);

		stream_ << "[";
		if (g_log_process_id)
			stream_ << CurrentProcessId() << ':';
		if (g_log_thread_id)
			stream_ << base::PlatformThread::CurrentId() << ':';
		if (g_log_timestamp) {
#if defined(OS_POSIX)
			timeval tv; // 秒、微秒
			gettimeofday(&tv, nullptr);
			time_t t = tv.tv_sec;

			struct tm local_time;
			localtime_r(&t, &local_time);
			struct tm* tm_time = &local_time;
			stream_ << std::setfill('0') << std::setw(2) << 1 + tm_time->tm_mon
				<< std::setw(2) << tm_time->tm_mday << '/'
				<< std::setw(2) << tm_time->tm_hour
				<< std::setw(2) << tm_time->tm_min
				<< std::setw(2) << tm_time->tm_sec << '.'
				<< std::setw(6) << tv.tv_usec << ':';
#elif defined(OS_WIN)
			// windows 日志时间
#endif
		}

		if (g_log_tickcount)
			stream_ << TickCount() << ':';

	}


	ErrnoLogMessage::ErrnoLogMessage(const char* file, int line, LogSeverity severity, SystemErrorCode err)
		:err_(err),
		log_message_(file, line, severity)
	{

	}


#endif
}


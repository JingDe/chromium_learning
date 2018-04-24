#include"base/logging.h"

#include<iomanip>

#include<unistd.h>
#include<sys/time.h>

#include"base/strings/string_piece.h"
#include"base/threading/platform_thread.h"



namespace logging {

	const char* const log_severity_names[] = {"INFO", "WARNING", "ERROR", "FATAL"};
	static_assert(LOG_NUM_SEVERITIES == arraysize(log_severity_names), "Incorrect number of log_severity_names");

	const char* log_severity_name(int severity)
	{
		if (severity >= 0 && severity < LOG_NUM_SEVERITIES)
			return log_severity_names[severity];
		return "UNKNOWN";
	}

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
		return GetTickCount(); // 系统启动的毫秒数
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

	LogMessage::LogMessage(const char* file, int line, const char* condition)
		:severity_(LOG_FATAL), file_(file), line_(line)
	{
		Init(file, line);
		stream_ << "Check failed: " << condition << ". ";
	}

	LogMessage::LogMessage(const char* file, int line, std::string* result)
		:severity_(LOG_FATAL), file_(file), line_(line)
	{
		Init(file, line);
		stream_ << "Check failed: " << *result;
		delete result;
	}

	LogMessage::LogMessage(const char* file, int line, LogSeverity severity, std::string* result)
		:severity_(severity), file_(file), line_(line)
	{
		Init(file, line);
		stream_ << "Check failed: " << *result;
		delete result;
	}

	// 日志头格式：[进程ID:线程ID:月日/时分秒.微秒:tick:等级:文件名(行号)]
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
			stream_ << TickCount() << ':'; // 系统启动的tick数

		if (severity_ >= 0)
			stream_ << log_severity_name(severity_);
		else
			stream_ << "VERBOSE" << -severity_;

		stream_ << ":" << filename.data() << "(" << line << ")]";

		message_start_ = stream_.str().length();
	}

	LogMessage::~LogMessage()
	{
		size_t stack_start = stream_.tellp(); // 流的当前位置
#if !defined(OFFICIAL_BUILD) && !defined(OS_NACL) && !defined(__UCLIBC__) && \
    !defined(OS_AIX)
		if (severity_ == LOG_FATAL && !base::debug::BeingDebugged())
		{
			base::debug::StackTrace trace;
			stream_ << std::endl;
			trace.OutputToStream(&stream_);
		}
#endif
		stream_ << std::endl;
		std::string str_newline(stream_.str());

		if (log_message_handler &&  log_message_handler(severity_, file_, line_, message_start_, str_newline))
		{
			return;
		}

		if ((g_logging_destination  &  LOG_TO_SYSTEM_DEBUG_LOG) != 0)
		{
			// ...
			ignore_result(fwrite(str_newline.data(), str_newling.size(), 1, stderr));
			fflush(stderr);
		}
		else if (severity_ >= kAlwaysPrintErrorLevel)
		{
			ignore_result(fwrite(str_newline.data(), str_newling.size(), 1, stderr));
			fflush(stderr);
		}

		// 写到日志文件
		if ((g_logging_destination & LOG_TO_FILE) != 0)
		{
#if !defined(OS_WIN)
			LoggingLock::Init(CLOCK_LOG_FILE, nullptr);
			LoggingLock logging_lock;
#endif
			if (InitializeLogFileHandle())
			{
#if defined(OS_WIN)
				//
#else
				ignore_result(fwrite(str_newline.data(), str_newling.size(), 1, g_log_file));
				fflush(g_log_file);
#endif
			}
		}

		if (severity_ == LOG_FATAL)
		{
			// 写到全局activity tracker

			if (log_assert_handler_stack.IsCreated() && !log_assert_handler_stack.Get().empty())
			{

			}
			else
			{
#ifndef NDEBUG
				if (!base::debug::BegingDebugged())
					DisplayDebugMessageInDialog(stream_.str());
#endif
				base::debug::BreakDebugger();
			}

		}
	}

	BASE_EXPORT std::string SystemErrorCodeToString(SystemErrorCode err_code)
	{
		return base::safe_strerror(error_code) + base::StringPrintf(" (%d)", error_code);
	}

	ErrnoLogMessage::ErrnoLogMessage(const char* file, int line, LogSeverity severity, SystemErrorCode err)
		:err_(err),
		log_message_(file, line, severity)
	{

	}

	ErrnoLogMessage::~ErrnoLogMessage()
	{
		stream() << ": " << SystemErrorCodeToString(err_);
		int last_error = err_;
		base::debug::Alias(&last_error);
	}

#endif
}


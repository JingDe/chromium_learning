#ifndef BASE_LOGGING_H_
#define BASE_LOGGING_H_

#include<iostream>
#include<string>
#include<sstream>

#include"base/base_export.h"
#include"base/strings/string_piece_forward.h"

namespace logging {

// 如果条件不满足，省去evaluating the arguments to a stream 
#define LAZY_STREAM(stream, condition) \
	!(condition) ? (void)0 : ::logging::LogMessageVoidify() & (stream)


// 组合日志消息类实例的宏
#define COMPACT_GOOGLE_LOG_EX_INFO(ClassName, ...) \
	::logging::ClassName(__FILE__, __LINE__, ::logging::LOG_INFO, ##__VA_ARGS__)
#define COMPACT_GOOGLE_LOG_EX_WARNING(ClassName, ...) \
	::logging::ClassName(__FILE__, __LINE__, ::logging::LOG_WARNING, ##__VA_ARGS__)
#define COMPACT_GOOGLE_LOG_EX_FATAL(ClassName, ...) \
	::logging::ClassName(__FILE__, __LINE__, ::logging::LOG_FATAL, ##__VA_ARGS__)


// PLOG_STREAM(severity) : 不同严重等级的日志类
#if defined(OS_WIN)
#define PLOG_STREAM(severity)
	COMPACT_GOOGLE_LOG_EX_ ## severity(Win32ErrorLogMessage, ::logging::GetLastSystemErrorCode()).stream()
#elif defined(OS_POSIX)
#define PLOG_STREAM(severity) \
	COMPACT_GOOGLE_LOG_EX_ ## severity(ErrnoLogMessage, ::logging::GetLastSystemErrorCode()).stream()
#endif


// ?
#if defined(__clang_analyzer__)
#define ANALYZER_ASSUME_TRUE(arg) logging::AnalyzerAssumeTrue(!!(arg))
#else
#define ANALYZER_ASSUME_TRUE(arg) (arg)
#endif


// 检查条件，输出日志
// PCHECK(condition) : condition不满足时，输出到stream
#if defined(OFFICIAL_BUILD)  &&  defined(NDEBUG)
#define PCHECK(condition)  \
LAZY_STREAM(PLOG_STREAM(FATAL), UNLIKELEY(!(condition))); \
EAT_STREAM_PARAMETERS
#else
#define PCHECK(condition) \ 
LAZY_STREAM(PLOG_STREAM(FATAL), !ANALYZER_ASSUME_TRUE(condition)) \
<<"Check failed: "#condition "."
#endif


// LogMessageVodify类用来明确忽略条件宏的值，避免编译器警告
// "value computed is not used" and "statement has no effect"
class LogMessageVoidify {
public:
	LogMessageVoidify() = default;

	void operator&(std::ostream&) {}
};


#if defined(OS_WIN)
typedef unsigned long SystemErrorCode;
#elif defined(OS_POSIX)
typedef int SystemErrorCode;
#endif


// 日志等级
typedef int LogSeverity;
const LogSeverity LOG_VERBOSE = -1;
const LogSeverity LOG_INFO = 0;
const LogSeverity LOG_LOG_WARNING = 1;
const LogSeverity LOG_ERROR = 2;
const LogSeverity LOG_FATAL = 3;
const LogSeverity LOG_NUM_SEVERITIES = 4;

BASE_EXPORT SystemErrorCode GetLastSystemErrorCode();

// 日志消息类:表示一条特定的日志消息
// 创建LogMessage实例，向其输入数据，完成输入后，析构函数被调用，所有数据被输出到何时的目的地
class BASE_EXPORT LogMessage {
public:
	LogMessage(const char* file, int line, LogSeverity severity);
	LogMessage(const char* file, int line, const char* condition); // CHECK()，默认LOG_FATAL
	LogMessage(const char* file, int line, std::string* result);
	LogMessage(const char* file, int line, LogSeverity severity, std::string* result);
	~LogMessage();

	std::ostream& stream() { return stream_; }
	std::string str() { return stream_.str(); }

private:
	void Init(const char* file, int line);

	LogSeverity severity_;
	std::ostringstream stream_;
	size_t message_start_;

	const char* file_;
	const int line_;

};

#if defined(OS_WIN)
class BASE_EXPORT W32ErrorLogMessage {
};
#elif defined(OS_POSIX)
class BASE_EXPORT ErrnoLogMessage {
public:
	ErrnoLogMessage(const char* file, int line, LogSeverity severity, SystemErrorCode err);
	~ErrnoLogMessage();

	std::ostream& stream() { return log_message_.stream(); }

private:
	SystemErrorCode err_;
	LogMessage log_message_;

};
#endif

}
#endif

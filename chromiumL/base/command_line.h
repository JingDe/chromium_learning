#ifndef BASE_COMMAND_LINE_H_
#define BASE_COMMAND_LINE_H_

#include<string>
#include<vector>
#include<map>
#include<functional>

#include"base/base_export.h"

namespace base {
	class FilePath;

	class BASE_EXPORT CommandLine 
	{
	public:

#if defined(OS_WIN)
		// The native command line string type.
		using StringType = string16; // std::wstring
#elif defined(OS_POSIX)
		using StringType = std::string;
#endif
		using CharType = StringType::value_type; // char
		using StringVector = std::vector<StringType>;
		using SwitchMap = std::map<std::string, StringType, std::less<std::string> >;

		enum NoProgram { NO_PROGRAM };
		explicit CommandLine(NoProgram no_program);

		static bool Init(int argc, const char* const* argv);

		void InitFromArgv(int argc, const CharType* const* argv);
		void InitFromArgv(const StringVector& argv);

		void SetProgram(const FilePath& program);

	private:

		static CommandLine* current_process_commandline_;

		// The argv array: { program, [(--|-|/)switch[=value]]*, [--], [argument]* }
		StringVector argv_; 

		// Parsed-out switch keys and values.
		SwitchMap switches_;

		// The index after the program and switches, any arguments start here.
		size_t begin_args_;
	};
}

#endif
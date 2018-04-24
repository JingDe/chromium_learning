#include"base/command_line.h"
#include"base/files/file_path.h"

namespace base {

	CommandLine::CommandLine(NoProgram no_program)
		:argv_(1),
		begin_args_(1) {

	}

	bool CommandLine::Init(int argc, const char* const* argv)
	{
		if (current_process_commandline_)
		{
			return false;
		}

		current_process_commandline_ = new CommandLine(NO_PROGRAM);
		current_process_commandline_->InitFromArgv(argc, argv);

		return true;
	}

	void CommandLine::InitFromArgv(int argc, const CommandLine::CharType* const* argv)
	{
		StringVector new_argv;
		for (int i = 0; i < argc; ++i)
			new_argv.push_back(argv[i]);
		InitFromArgv(new_argv);
	}

	void CommandLine::InitFromArgv(const StringVector& argv) // vector<std::string>
	{
		argv_ = StringVector(1);
		switches_.clear();
		begin_args_ = 1;
		SetProgram(argv.empty() ? FilePath() : FilePath(argv[0]));
		AppendSwitchesAndArguments(this, argv);
	}

	void CommandLine::SetProgram(const FilePath& program)
	{
		TrimWhitespaceASCII(program.value(), TRIM_ALL, &argv_[0]);
	}
}
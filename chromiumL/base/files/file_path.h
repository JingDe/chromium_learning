#ifndef BASE_FILES_FILE_PATH_H_
#define BASE_FILES_FILE_PATH_H_

#include "base/strings/string_piece.h"

namespace base {
	class FilePath {
	public:
#if defined(OS_POSIX)
		// On most platforms, native pathnames are char arrays, and the encoding
		// may or may not be specified.  On Mac OS X, native pathnames are encoded
		// in UTF-8.
		typedef std::string StringType;
#elif defined(OS_WIN)
		// On Windows, for Unicode-aware applications, native pathnames are wchar_t
		// arrays encoded in UTF-16.
		typedef std::wstring StringType;
#endif  // OS_WIN

		typedef BasicStringPiece<StringType> StringPieceType;

		FilePath();
		FilePath(const FilePath& that);
		FilePath(FilePath&& that) noexcept; // ²»Å×³öÒì³£
		explicit FilePath(StringPieceType path);

		const StringType& value() const { return path_; }

	private:
		StringType path_;
	};

}

#endif
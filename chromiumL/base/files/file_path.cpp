#include"file_path.h"

namespace base {
	FilePath::FilePath() = default;

	FilePath::FilePath(const FilePath& that) = default;

	FilePath::FilePath(FilePath&&  that) noexcept = default;

}
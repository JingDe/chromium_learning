#ifndef BASE_FILES_FILE_UTIL_H_
#define BASE_FILES_FILE_UTIL_H_

#include"base/base_export.h"
//

class FilePath;

namespace base {

	//获得当前进程的工作目录
	BASE_EXPORT bool GetCurrentDirectory(FilePath* path);
}

#endif
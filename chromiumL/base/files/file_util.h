#ifndef BASE_FILES_FILE_UTIL_H_
#define BASE_FILES_FILE_UTIL_H_

#include"base/base_export.h"
//

class FilePath;

namespace base {

	//��õ�ǰ���̵Ĺ���Ŀ¼
	BASE_EXPORT bool GetCurrentDirectory(FilePath* path);
}

#endif
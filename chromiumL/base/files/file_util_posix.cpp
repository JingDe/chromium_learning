#include"file_util.h"

#include<unistd.h>
#include<limits.h> // PATH_MAX 4096

#include"base/files/file_path.h"
#include"base/logging.h"


namespace base {


	bool GetCurrentDirectory(FilePath* dir)
	{
		// AssertBlockingAllowed();

		char system_buffer[PATH_MAX] = "";
		if (!getcwd(system_buffer, sizeof system_buffer)) // getcwd获得当前工作目录
		{
			NOTREACHED();
			return false;
		}
		*dir = FilePath(system_buffer);
		return true;
	}
}
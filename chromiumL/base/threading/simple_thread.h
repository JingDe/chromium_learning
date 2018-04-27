#ifndef BASE_THREADING_SIMPLE_THREAD_H_
#define BASE_THREADING_SIMPLE_THREAD_H_

#include "base/base_export.h"
#include "base/threading/platform_thread.h"

namespace base {

class BASE_EXPORT SimpleThread : public PlatformThread::Delegate {

};

}

#endif

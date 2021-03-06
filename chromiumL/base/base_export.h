#ifndef BASE_BASE_EXPORT_H_
#define BASE_BASE_EXPORT_H_

#if defined(COMPONENT_BUILD)
	#if defined(WIN32)
		#if defined(BASE_IMPLEMENTATION)
		#define BASE_EXPORT __declspec(dllexport)
		#else
		#define BASE_EXPORT __declspec(dllimport)
		#endif
	#else
		#if defined(BASE_IMPLEMENTATION)
		#define BASE_EXPORT __attribute__((visibility("default")))
		#else
		#define BASE_EXPORT
		#endif
	#endif

#else
	#define BASE_EXPORT
#endif

#endif

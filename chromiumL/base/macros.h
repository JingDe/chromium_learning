#ifndef BASE_MACROS_H_
#define BASE_MACROS_H_

// ×èÖ¹¿½±´¹¹Ôì
#define DISALLOW_COPY(TypeName) \
	TypeName(const TypeName&)=delete

// ×èÖ¹¿½±´¸³Öµ
#define DISALLOW_ASSIGN(TypeName) \
	TypeName& operator=(const TypeName&)=delete

// ×èÖ¹¿½±´
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	DISALLOW_COPY(TypeName); \
	DISALLOW_ASSIGN(TypeName)

// ×èÖ¹Ä¬ÈÏ¹¹Ôìº¯Êý¡¢¿½±´
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
	TypeName()=delete; \
	DISALLOW_COPY_AND_ASSIGN(TypeName)

#endif
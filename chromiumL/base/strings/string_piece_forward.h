#ifndef BASE_STRINGS_STRING_PIECE_FORWARD_H_
#define BASE_STRINGS_STRING_PIECE_FORWARD_H_

#include<string>

namespace base {
	template<typename STRING_TYPE>
	class BasicStringPiece;
	typedef BasicStringPiece<std::string> StringPiece;
}

#endif

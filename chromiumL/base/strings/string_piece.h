#ifndef BASE_STRING_PIECE_H_
#define BASE_STRING_PIECE_H_

namespace base {
	template <typename STRING_TYPE> class BasicStringPiece {
	public:
		// Standard STL container boilerplate.
		typedef size_t size_type;
		typedef typename STRING_TYPE::value_type value_type; // std::string::value_type «char
		typedef const value_type* pointer;
		typedef const value_type& reference;
		typedef const value_type& const_reference;
		typedef ptrdiff_t difference_type;
		typedef const value_type* const_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		static const size_type npos;

	public:
		// We provide non-explicit singleton constructors so users can pass
		// in a "const char*" or a "string" wherever a "StringPiece" is
		// expected (likewise for char16, string16, StringPiece16).
		constexpr BasicStringPiece() : ptr_(NULL), length_(0) {}
		BasicStringPiece(const value_type* str)
			: ptr_(str),
			length_((str == NULL) ? 0 : STRING_TYPE::traits_type::length(str)) {}
		BasicStringPiece(const STRING_TYPE& str)
			: ptr_(str.data()), length_(str.size()) {}
		constexpr BasicStringPiece(const value_type* offset, size_type len)
			: ptr_(offset), length_(len) {}
		BasicStringPiece(const typename STRING_TYPE::const_iterator& begin,
			const typename STRING_TYPE::const_iterator& end) {
#if DCHECK_IS_ON()
			// This assertion is done out-of-line to avoid bringing in logging.h and
			// instantiating logging macros for every instantiation.
			internal::AssertIteratorsInOrder(begin, end);
#endif
			length_ = static_cast<size_t>(std::distance(begin, end));

			// The length test before assignment is to avoid dereferencing an iterator
			// that may point to the end() of a string.
			ptr_ = length_ > 0 ? &*begin : nullptr;
		}

		// data() may return a pointer to a buffer with embedded NULs, and the
		// returned buffer may or may not be null terminated.  Therefore it is
		// typically a mistake to pass data() to a routine that expects a NUL
		// terminated string.
		constexpr const value_type* data() const { return ptr_; }
		constexpr size_type size() const { return length_; }
		constexpr size_type length() const { return length_; }
		bool empty() const { return length_ == 0; }

		void clear() {
			ptr_ = NULL;
			length_ = 0;
		}
		void set(const value_type* data, size_type len) {
			ptr_ = data;
			length_ = len;
		}
		void set(const value_type* str) {
			ptr_ = str;
			length_ = str ? STRING_TYPE::traits_type::length(str) : 0;
		}

		constexpr value_type operator[](size_type i) const {
			CHECK(i < length_);
			return ptr_[i];
		}

		value_type front() const {
			CHECK_NE(0UL, length_);
			return ptr_[0];
		}

		value_type back() const {
			CHECK_NE(0UL, length_);
			return ptr_[length_ - 1];
		}

		constexpr void remove_prefix(size_type n) {
			CHECK(n <= length_);
			ptr_ += n;
			length_ -= n;
		}

		constexpr void remove_suffix(size_type n) {
			CHECK(n <= length_);
			length_ -= n;
		}

		int compare(const BasicStringPiece<STRING_TYPE>& x) const {
			int r = wordmemcmp(
				ptr_, x.ptr_, (length_ < x.length_ ? length_ : x.length_));
			if (r == 0) {
				if (length_ < x.length_) r = -1;
				else if (length_ > x.length_) r = +1;
			}
			return r;
		}

		// This is the style of conversion preferred by std::string_view in C++17.
		explicit operator STRING_TYPE() const { return as_string(); }

		STRING_TYPE as_string() const {
			// std::string doesn't like to take a NULL pointer even with a 0 size.
			return empty() ? STRING_TYPE() : STRING_TYPE(data(), size());
		}

		const_iterator begin() const { return ptr_; }
		const_iterator end() const { return ptr_ + length_; }
		const_reverse_iterator rbegin() const {
			return const_reverse_iterator(ptr_ + length_);
		}
		const_reverse_iterator rend() const {
			return const_reverse_iterator(ptr_);
		}

		size_type max_size() const { return length_; }
		size_type capacity() const { return length_; }

		static int wordmemcmp(const value_type* p,
			const value_type* p2,
			size_type N) {
			return STRING_TYPE::traits_type::compare(p, p2, N);
		}

		// Sets the value of the given string target type to be the current string.
		// This saves a temporary over doing |a = b.as_string()|
		void CopyToString(STRING_TYPE* target) const {
			internal::CopyToString(*this, target);
		}

		void AppendToString(STRING_TYPE* target) const {
			internal::AppendToString(*this, target);
		}

		size_type copy(value_type* buf, size_type n, size_type pos = 0) const {
			return internal::copy(*this, buf, n, pos);
		}

		// Does "this" start with "x"
		bool starts_with(const BasicStringPiece& x) const {
			return ((this->length_ >= x.length_) &&
				(wordmemcmp(this->ptr_, x.ptr_, x.length_) == 0));
		}

		// Does "this" end with "x"
		bool ends_with(const BasicStringPiece& x) const {
			return ((this->length_ >= x.length_) &&
				(wordmemcmp(this->ptr_ + (this->length_ - x.length_),
					x.ptr_, x.length_) == 0));
		}

		// find: Search for a character or substring at a given offset.
		size_type find(const BasicStringPiece<STRING_TYPE>& s,
			size_type pos = 0) const {
			return internal::find(*this, s, pos);
		}
		size_type find(value_type c, size_type pos = 0) const {
			return internal::find(*this, c, pos);
		}

		// rfind: Reverse find.
		size_type rfind(const BasicStringPiece& s,
			size_type pos = BasicStringPiece::npos) const {
			return internal::rfind(*this, s, pos);
		}
		size_type rfind(value_type c, size_type pos = BasicStringPiece::npos) const {
			return internal::rfind(*this, c, pos);
		}

		// find_first_of: Find the first occurence of one of a set of characters.
		size_type find_first_of(const BasicStringPiece& s,
			size_type pos = 0) const {
			return internal::find_first_of(*this, s, pos);
		}
		size_type find_first_of(value_type c, size_type pos = 0) const {
			return find(c, pos);
		}

		// find_first_not_of: Find the first occurence not of a set of characters.
		size_type find_first_not_of(const BasicStringPiece& s,
			size_type pos = 0) const {
			return internal::find_first_not_of(*this, s, pos);
		}
		size_type find_first_not_of(value_type c, size_type pos = 0) const {
			return internal::find_first_not_of(*this, c, pos);
		}

		// find_last_of: Find the last occurence of one of a set of characters.
		size_type find_last_of(const BasicStringPiece& s,
			size_type pos = BasicStringPiece::npos) const {
			return internal::find_last_of(*this, s, pos);
		}
		size_type find_last_of(value_type c,
			size_type pos = BasicStringPiece::npos) const {
			return rfind(c, pos);
		}

		// find_last_not_of: Find the last occurence not of a set of characters.
		size_type find_last_not_of(const BasicStringPiece& s,
			size_type pos = BasicStringPiece::npos) const {
			return internal::find_last_not_of(*this, s, pos);
		}
		size_type find_last_not_of(value_type c,
			size_type pos = BasicStringPiece::npos) const {
			return internal::find_last_not_of(*this, c, pos);
		}

		// substr.
		BasicStringPiece substr(size_type pos,
			size_type n = BasicStringPiece::npos) const {
			return internal::substr(*this, pos, n);
		}

	protected:
		const value_type* ptr_;
		size_type length_;
	};


}
#endif
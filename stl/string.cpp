#ifndef USE_STL
#include <stl/string.h>
#include <stl/string.hpp>

namespace stl {

	template class basic_string<TINYSTL_ALLOCATOR>;

	template MUD_INFRA_EXPORT string operator+(const string& lhs, const string& rhs);
	template MUD_INFRA_EXPORT string operator+(const char* lhs, const string& rhs);
	template MUD_INFRA_EXPORT string operator+(const string& lhs, const char* rhs);
}
#endif

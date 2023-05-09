#ifndef UTIL_OVERLOADED_HPP
#define UTIL_OVERLOADED_HPP

namespace Util{
	// helper type for the std::variant visitor
	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	// explicit deduction guide (not needed as of C++20)
	template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}

#endif //UTIL_OVERLOADED_HPP

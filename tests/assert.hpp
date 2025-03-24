#pragma once

#include <cstdlib>
#include <iostream>
#include <source_location>
#include <sstream>

#define NALCHI_TESTS_ASSERT(condition, ...) \
    do \
    { \
        if (!(condition)) [[unlikely]] \
        { \
            std::ostringstream oss; \
            const auto loc = std::source_location::current(); \
            oss << "NALCHI_TEST_ASSERT failed: " << #condition << '\n'; \
            oss << "\tin " << loc.file_name() << ':' << loc.line() << ':' << loc.column() << '\n'; \
            __VA_OPT__(oss << '\t';) \
            append_args(oss __VA_OPT__(, ) __VA_ARGS__); \
            std::cout << oss.str() << std::endl; \
            std::exit(1); \
        } \
    } while (false)

namespace nalchi::tests
{

template <typename... Args>
auto append_args(std::ostream& os, const Args&... args) -> std::ostream&
{
    if constexpr (sizeof...(args) > 0)
        (os << ... << args);

    return os;
}

} // namespace nalchi::tests

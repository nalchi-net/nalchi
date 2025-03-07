#pragma once

#include <string>

namespace nalchi
{

template <typename T>
concept character = requires { typename std::char_traits<T>; };

} // namespace nalchi

#pragma once

#include <concepts>
#include <string>

namespace nalchi
{

template <typename T>
concept character = std::same_as<T, char> || std::same_as<T, wchar_t> || std::same_as<T, char8_t> ||
                    std::same_as<T, char16_t> || std::same_as<T, char32_t>;

} // namespace nalchi

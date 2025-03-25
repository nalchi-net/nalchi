#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>

namespace nalchi
{

template <typename R, typename T>
concept typed_input_range = std::ranges::input_range<R> &&
                            std::same_as<T, std::remove_cvref_t<decltype(*std::ranges::begin(std::declval<R>()))>>;

}

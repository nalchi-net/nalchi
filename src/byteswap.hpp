#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <type_traits>

namespace nalchi
{

template <typename Num>
    requires std::is_arithmetic_v<Num>
static Num byteswap(Num value) noexcept
{
    static_assert(std::has_unique_object_representations_v<Num>, "`Num` may not have padding bits");

    auto value_representation = std::bit_cast<std::array<std::byte, sizeof(Num)>>(value);
    std::ranges::reverse(value_representation);
    return std::bit_cast<Num>(value_representation);
}

} // namespace nalchi

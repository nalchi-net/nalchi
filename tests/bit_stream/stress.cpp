#include <nalchi/bit_stream.hpp>

#include "private_accessors.hpp"

#include "../assert.hpp"

#include <steam/steamnetworkingtypes.h>

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#ifndef BS_ITERATIONS
#define BS_ITERATIONS 1000
#endif

#define BS_ASSERT(condition, ...) \
    NALCHI_TESTS_ASSERT(condition, "seed = ", seed, ", size = ", logical_bytes_length, \
                        '\n' __VA_OPT__(, '\t', ) __VA_ARGS__, __VA_OPT__('\n', ) get_inputs_oss().rdbuf())

#define BS_ASSERT_WRITER_INVARIANTS \
    do \
    { \
        const auto fail = get_writer_fail(writer); \
        BS_ASSERT(!fail, "writer fail flag set"); \
\
        const auto scratch = get_writer_scratch(writer); \
        constexpr auto scratch_max = std::numeric_limits<bit_stream_writer::word_type>::max(); \
        BS_ASSERT(scratch <= scratch_max, "writer scratch = ", scratch, " exceeded ", scratch_max); \
\
        const auto scratch_index = get_writer_scratch_index(writer); \
        constexpr auto scratch_index_max = std::bit_width(scratch_max); \
        BS_ASSERT(0 <= scratch_index && scratch_index < scratch_index_max, "writer scratch index = ", scratch_index, \
                  " out of range [0, ", scratch_index_max, ')'); \
\
        const auto words_index = get_writer_words_index(writer); \
        BS_ASSERT(0 <= words_index && static_cast<std::size_t>(words_index) <= logical_bytes_length, \
                  "writer words index = ", words_index, " out of range [0, ", logical_bytes_length, ']'); \
\
        const auto logical_total_bits = get_writer_logical_total_bits(writer); \
        const auto logical_used_bits = get_writer_logical_used_bits(writer); \
        BS_ASSERT(logical_used_bits == static_cast<std::size_t>(scratch_index_max * words_index + scratch_index), \
                  "writer logical used bits = ", logical_used_bits, " mismatch with ", scratch_index_max, \
                  " * words index(", words_index, ") + scratch index(", scratch_index, \
                  ") = ", scratch_index_max * words_index + scratch_index); \
        BS_ASSERT(logical_used_bits <= logical_total_bits, "writer logical used bits = ", logical_used_bits, \
                  " exceeded logical total bits = ", logical_total_bits); \
    } while (false)

#define BS_ASSERT_READER_INVARIANTS \
    do \
    { \
        const auto fail = get_reader_fail(reader); \
        BS_ASSERT(!fail, "reader fail flag set"); \
\
        const auto scratch = get_reader_scratch(reader); \
        constexpr auto scratch_max = std::numeric_limits<bit_stream_reader::word_type>::max(); \
        BS_ASSERT(scratch <= scratch_max, "reader scratch = ", scratch, " exceeded ", scratch_max); \
\
        const auto scratch_bits = get_reader_scratch_bits(reader); \
        constexpr auto scratch_bits_max = std::bit_width(scratch_max); \
        BS_ASSERT(0 <= scratch_bits && scratch_bits < scratch_bits_max, "reader scratch bits = ", scratch_bits, \
                  " out of range [0, ", scratch_bits_max, ')'); \
\
        const auto words_index = get_reader_words_index(reader); \
        BS_ASSERT(0 <= words_index && static_cast<std::size_t>(words_index) <= logical_bytes_length, \
                  "reader words index = ", words_index, " out of range [0, ", logical_bytes_length, ']'); \
\
        const auto logical_total_bits = get_reader_logical_total_bits(reader); \
        const auto logical_used_bits = get_reader_logical_used_bits(reader); \
        BS_ASSERT(logical_used_bits <= logical_total_bits, "reader logical used bits = ", logical_used_bits, \
                  " exceeded logical total bits = ", logical_total_bits); \
    } while (false)

#define BS_WRITER_INPUT_LIMITED_INT(int_type) \
    do \
    { \
        using UInt = std::make_unsigned_t<int_type>; \
\
        /* Generate input (value, min, max) */ \
        std::array<int_type, 3> nums; \
        for (auto& num : nums) \
            num = static_cast<int_type>(get_int_distribution_full<int_type>()(rng)); \
        if (nums[0] == nums[1] && nums[1] == nums[2]) \
            continue; \
        std::ranges::sort(nums); \
        limited_int<int_type> input{ \
            .value = nums[1], \
            .min = nums[0], \
            .max = nums[2], \
        }; \
\
        /* Calculate bit width of (min, max), and backup if overflow expected */ \
        const int bits = std::bit_width(static_cast<UInt>(((UInt)input.max) - ((UInt)input.min))); \
        overflow_expected = (writer.used_bits() + bits > writer.total_bits()); \
        if (overflow_expected) \
            backup_writer_states(); \
\
        /* Write results */ \
        writer.write(input.value, input.min, input.max); \
        g_inputs.push_back(input); \
    } while (false)

#define BS_WRITER_INPUT_CHAR(char_type) \
    do \
    { \
        using UInt = std::make_unsigned_t<char_type>; \
\
        /* Generate input value */ \
        const char_type value = static_cast<char_type>(get_int_distribution_full<UInt>()(rng)); \
\
        /* Backup if overflow expected */ \
        overflow_expected = (writer.used_bits() + 8 * sizeof(char_type) > writer.total_bits()); \
        if (overflow_expected) \
            backup_writer_states(); \
\
        /* Write results */ \
        writer.write(value); \
        g_inputs.push_back(value); \
    } while (false)

#define BS_WRITER_INPUT_REAL(float_type) \
    do \
    { \
        /* Generate input value */ \
        const float_type value = get_real_distribution_full<float_type>()(rng); \
\
        /* Backup if overflow expected */ \
        overflow_expected = (writer.used_bits() + 8 * sizeof(float_type) > writer.total_bits()); \
        if (overflow_expected) \
            backup_writer_states(); \
\
        /* Write results */ \
        writer.write(value); \
        g_inputs.push_back(value); \
    } while (false)

#define BS_WRITER_INPUT_STR(str_type) \
    do \
    { \
        /* Generate input length */ \
        const size_type max_len = std::min( \
            INPUT_STR_MAX_LEN, 1 + writer.unused_bytes() / static_cast<size_type>(sizeof(str_type::value_type))); \
        const size_type len = std::uniform_int_distribution<size_type>(1, max_len)(rng); \
\
        /* String bit calculation is too messy, so we just continues for the potential overflows */ \
        if (1 + sizeof(std::uint32_t) + len * sizeof(str_type::value_type) > writer.unused_bytes()) \
            continue; \
\
        /* Generate input value */ \
        str_type value; \
        value.reserve(len); \
        auto dist = get_int_distribution_full<std::make_unsigned_t<str_type::value_type>>(); \
        for (size_type i = 0; i < len; ++i) \
            value.push_back(static_cast<str_type::value_type>(dist(rng))); \
\
        /* No overflow for the string! */ \
\
        /* Write results */ \
        writer.write(value); \
        g_inputs.push_back(value); \
    } while (false)

namespace nalchi::tests
{

using rng_type = std::mt19937_64;
using seed_type = rng_type::result_type;

using word_type = bit_stream_writer::word_type;
using size_type = bit_stream_writer::size_type;

struct user_data
{
    std::int8_t s8;
    std::int64_t s64;
    std::uint16_t u16;
    void* ptr;
    std::int32_t s32;

    constexpr bool operator==(const user_data&) const = default;

    friend auto operator<<(std::ostream& os, const user_data& data) -> std::ostream&
    {
        os << "user_data {s8=" << data.s8 << ", s64=" << data.s64 << ", u16=" << data.u16 << ", s32=" << data.s32
           << "}";
        return os;
    }
};

template <std::integral Int>
struct limited_int
{
    using value_type = Int;

    Int value;
    Int min;
    Int max;
};

using input =
    std::variant<user_data, bool, limited_int<std::int8_t>, limited_int<std::int16_t>, limited_int<std::int32_t>,
                 limited_int<std::int64_t>, limited_int<std::uint8_t>, limited_int<std::uint16_t>,
                 limited_int<std::uint32_t>, limited_int<std::uint64_t>, char, wchar_t, char8_t, char16_t, char32_t,
                 float, double, std::string, std::wstring, std::u8string, std::u16string, std::u32string>;
constexpr auto input_types = std::variant_size_v<input>;

template <std::integral Int>
    requires(sizeof(Int) != 1)
auto get_int_distribution_full() -> std::uniform_int_distribution<Int>
{
    return std::uniform_int_distribution<Int>(std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max());
}

template <std::integral Int>
    requires(sizeof(Int) == 1)
auto get_int_distribution_full()
    -> std::uniform_int_distribution<std::conditional_t<std::is_signed_v<Int>, int, unsigned>>
{
    if constexpr (std::is_signed_v<Int>)
        return std::uniform_int_distribution<int>(std::numeric_limits<Int>::min(), std::numeric_limits<Int>::max());
    else
        return std::uniform_int_distribution<unsigned>(std::numeric_limits<Int>::min(),
                                                       std::numeric_limits<Int>::max());
}

template <std::floating_point Float>
auto get_real_distribution_full() -> std::uniform_real_distribution<Float>
{
    return std::uniform_real_distribution<Float>(std::numeric_limits<Float>::min(), std::numeric_limits<Float>::max());
}

constexpr auto GNS_MAX_MSG_SEND_SIZE = ::k_cbMaxSteamNetworkingSocketsMessageSizeSend;
static_assert(GNS_MAX_MSG_SEND_SIZE % sizeof(word_type) == 0);

constexpr auto GNS_KNOWN_DEFAULT_MTU = 1200;

namespace
{

constexpr size_type INPUT_STR_MAX_LEN = 32;

std::vector<input> g_inputs(GNS_MAX_MSG_SEND_SIZE);
word_type g_buffer[GNS_MAX_MSG_SEND_SIZE / sizeof(word_type)];

} // namespace

/// @brief Gets the inputs as a `std::ostringstream` if something goes wrong.
auto get_inputs_oss() -> std::ostringstream
{
    std::ostringstream result;

    auto visitor = [&result](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, user_data>)
            result << arg << "}\n";
        else if constexpr (std::is_same_v<T, bool>)
            result << "bool " << std::boolalpha << arg << std::noboolalpha << '\n';
        else if constexpr (std::is_same_v<T, limited_int<std::int8_t>>)
            result << "limited_s8 {value=" << arg.value << ", min=" << arg.min << ", max=" << arg.max << "}\n";
        else if constexpr (std::is_same_v<T, limited_int<std::int16_t>>)
            result << "limited_s16 {value=" << arg.value << ", min=" << arg.min << ", max=" << arg.max << "}\n";
        else if constexpr (std::is_same_v<T, limited_int<std::int32_t>>)
            result << "limited_s32 {value=" << arg.value << ", min=" << arg.min << ", max=" << arg.max << "}\n";
        else if constexpr (std::is_same_v<T, limited_int<std::int64_t>>)
            result << "limited_s64 {value=" << arg.value << ", min=" << arg.min << ", max=" << arg.max << "}\n";
        else if constexpr (std::is_same_v<T, limited_int<std::uint8_t>>)
            result << "limited_u8 {value=" << arg.value << ", min=" << arg.min << ", max=" << arg.max << "}\n";
        else if constexpr (std::is_same_v<T, limited_int<std::uint16_t>>)
            result << "limited_u16 {value=" << arg.value << ", min=" << arg.min << ", max=" << arg.max << "}\n";
        else if constexpr (std::is_same_v<T, limited_int<std::uint32_t>>)
            result << "limited_u32 {value=" << arg.value << ", min=" << arg.min << ", max=" << arg.max << "}\n";
        else if constexpr (std::is_same_v<T, limited_int<std::uint64_t>>)
            result << "limited_u64 {value=" << arg.value << ", min=" << arg.min << ", max=" << arg.max << "}\n";
        else if constexpr (std::is_same_v<T, char>)
            result << "char 0x" << std::hex << static_cast<std::uint32_t>(arg) << std::dec << "\n";
        else if constexpr (std::is_same_v<T, wchar_t>)
            result << "wchar_t 0x" << std::hex << static_cast<std::uint32_t>(arg) << std::dec << "\n";
        else if constexpr (std::is_same_v<T, char8_t>)
            result << "char8_t 0x" << std::hex << static_cast<std::uint32_t>(arg) << std::dec << "\n";
        else if constexpr (std::is_same_v<T, char16_t>)
            result << "char16_t 0x" << std::hex << static_cast<std::uint32_t>(arg) << std::dec << "\n";
        else if constexpr (std::is_same_v<T, char32_t>)
            result << "char32_t 0x" << std::hex << static_cast<std::uint32_t>(arg) << std::dec << "\n";
        else if constexpr (std::is_same_v<T, float>)
            result << "float " << arg << "\n";
        else if constexpr (std::is_same_v<T, double>)
            result << "double " << arg << "\n";
        else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring> ||
                           std::is_same_v<T, std::u8string> || std::is_same_v<T, std::u16string> ||
                           std::is_same_v<T, std::u32string>)
        {
            if constexpr (std::is_same_v<T, std::string>)
                result << "std::string ";
            else if constexpr (std::is_same_v<T, std::wstring>)
                result << "std::wstring ";
            else if constexpr (std::is_same_v<T, std::u8string>)
                result << "std::u8string ";
            else if constexpr (std::is_same_v<T, std::u16string>)
                result << "std::u16string ";
            else if constexpr (std::is_same_v<T, std::u32string>)
                result << "std::u32string ";
            result << std::hex;
            for (const auto ch : arg)
            {
                result << "0x" << static_cast<std::uint32_t>(ch) << " ";
            }
            result << std::dec << "\n";
        }
        else
            static_assert(false, "Invalid input type");
    };

    for (const auto& input : g_inputs)
        std::visit(visitor, input);

    return result;
}

/// @brief Tests both writing and reading via comparing input and output.
/// @param seed Internal seed to run the rng.
/// @param logical_bytes_length Number of bytes the bit stream will use.
void test_write_and_read(const seed_type seed, const size_type logical_bytes_length, bit_stream_writer& writer,
                         bit_stream_reader& reader)
{
    g_inputs.clear();

    writer.reset_with(g_buffer, logical_bytes_length);
    BS_ASSERT_WRITER_INVARIANTS;

    rng_type rng(seed);

    // Backup prev states to recover from writer fail
    bit_stream_writer::scratch_type prev_writer_scratch;
    std::span<bit_stream_writer::word_type> prev_writer_words;
    int prev_writer_scratch_index;
    int prev_writer_words_index;
    bit_stream_writer::size_type prev_writer_logical_total_bits;
    bit_stream_writer::size_type prev_writer_logical_used_bits;
    bool prev_writer_init_fail;
    bool prev_writer_fail;
    bool prev_writer_final_flushed;

    auto backup_writer_states = [&] {
        prev_writer_scratch = get_writer_scratch(writer);
        prev_writer_words = get_writer_words(writer);
        prev_writer_scratch_index = get_writer_scratch_index(writer);
        prev_writer_words_index = get_writer_words_index(writer);
        prev_writer_logical_total_bits = get_writer_logical_total_bits(writer);
        prev_writer_logical_used_bits = get_writer_logical_used_bits(writer);
        prev_writer_init_fail = get_writer_init_fail(writer);
        prev_writer_fail = get_writer_fail(writer);
        prev_writer_final_flushed = get_writer_final_flushed(writer);
    };

    auto restore_writer_states = [&] {
        get_writer_scratch(writer) = prev_writer_scratch;
        get_writer_words(writer) = prev_writer_words;
        get_writer_scratch_index(writer) = prev_writer_scratch_index;
        get_writer_words_index(writer) = prev_writer_words_index;
        get_writer_logical_total_bits(writer) = prev_writer_logical_total_bits;
        get_writer_logical_used_bits(writer) = prev_writer_logical_used_bits;
        get_writer_init_fail(writer) = prev_writer_init_fail;
        get_writer_fail(writer) = prev_writer_fail;
        get_writer_final_flushed(writer) = prev_writer_final_flushed;
    };

    // Fill the buffer with the writer
    std::uniform_int_distribution<std::size_t> input_types_dist(0, input_types - 1);
    while (writer.unused_bits() > 0)
    {
        bool overflow_expected = false;

        const auto input_type_index = input_types_dist(rng);
        switch (input_type_index)
        {
        case 0: // user_data
        {
            // Generate input value
            const user_data value{
                .s8 = static_cast<std::int8_t>(get_int_distribution_full<std::int8_t>()(rng)),
                .s64 = get_int_distribution_full<std::int64_t>()(rng),
                .u16 = get_int_distribution_full<std::uint16_t>()(rng),
                .ptr = nullptr,
                .s32 = get_int_distribution_full<std::int32_t>()(rng),
            };

            // Backup if overflow expected
            overflow_expected = (writer.used_bits() + 8 * sizeof(user_data) > writer.total_bits());
            if (overflow_expected)
                backup_writer_states();

            // Write results
            writer.write(&value, static_cast<size_type>(sizeof(user_data)));
            g_inputs.push_back(value);
            break;
        }
        case 1: // bool
        {
            // Generate input value
            const bool value = static_cast<bool>(get_int_distribution_full<bool>()(rng));

            // No overflow for bool! (there's at least 1 unused bit)

            // Write results
            writer.write(value);
            g_inputs.push_back(value);
            break;
        }
        case 2: // limited s8
            BS_WRITER_INPUT_LIMITED_INT(std::int8_t);
            break;
        case 3: // limited s16
            BS_WRITER_INPUT_LIMITED_INT(std::int16_t);
            break;
        case 4: // limited s32
            BS_WRITER_INPUT_LIMITED_INT(std::int32_t);
            break;
        case 5: // limited s64
            BS_WRITER_INPUT_LIMITED_INT(std::int64_t);
            break;
        case 6: // limited u8
            BS_WRITER_INPUT_LIMITED_INT(std::uint8_t);
            break;
        case 7: // limited u16
            BS_WRITER_INPUT_LIMITED_INT(std::uint16_t);
            break;
        case 8: // limited u32
            BS_WRITER_INPUT_LIMITED_INT(std::uint32_t);
            break;
        case 9: // limited u64
            BS_WRITER_INPUT_LIMITED_INT(std::uint64_t);
            break;
        case 10: // char
            BS_WRITER_INPUT_CHAR(char);
            break;
        case 11: // wchar_t
            BS_WRITER_INPUT_CHAR(wchar_t);
            break;
        case 12: // char8_t
            BS_WRITER_INPUT_CHAR(char8_t);
            break;
        case 13: // char16_t
            BS_WRITER_INPUT_CHAR(char16_t);
            break;
        case 14: // char32_t
            BS_WRITER_INPUT_CHAR(char32_t);
            break;
        case 15: // float
            BS_WRITER_INPUT_REAL(float);
            break;
        case 16: // double
            BS_WRITER_INPUT_REAL(double);
            break;
        case 17: // std::string
            BS_WRITER_INPUT_STR(std::string);
            break;
        case 18: // std::wstring
            BS_WRITER_INPUT_STR(std::wstring);
            break;
        case 19: // std::u8string
            BS_WRITER_INPUT_STR(std::u8string);
            break;
        case 20: // std::u16string
            BS_WRITER_INPUT_STR(std::u16string);
            break;
        case 21: // std::u32string
            BS_WRITER_INPUT_STR(std::u32string);
            break;
        default:
            BS_ASSERT(false, "Unhandled input type index = ", input_type_index);
            break;
        }

        BS_ASSERT(writer.fail() == overflow_expected, "writer fail = ", std::boolalpha, writer.fail(),
                  ", but overflow expected = ", overflow_expected, std::noboolalpha);

        if (overflow_expected)
        {
            // Revert to last successful input
            restore_writer_states();
            g_inputs.pop_back();

            break;
        }
        else
        {
            BS_ASSERT_WRITER_INVARIANTS;
        }
    }

    // Write done, flush to the buffer
    BS_ASSERT(writer.flush_final(), "writer flush failed");

    // Time to read from the buffer
    reader.reset_with(g_buffer, logical_bytes_length);
    BS_ASSERT_READER_INVARIANTS;

    int read_item_index = 0;

    // Prepare the visitor
    auto visitor = [&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, user_data>)
        {
            user_data value;
            BS_ASSERT(reader.read(&value, static_cast<size_type>(sizeof(user_data))), "read #", read_item_index,
                      " user data read failed");
            BS_ASSERT(value == arg, "read #", read_item_index, " expected = ", arg, ", got = ", value);
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            bool value;
            BS_ASSERT(reader.read(value), "read #", read_item_index, " bool read failed");
            BS_ASSERT(value == arg, "read #", read_item_index, " bool expected = ", std::boolalpha, arg,
                      ", got = ", value, std::noboolalpha);
        }
        else if constexpr (std::is_same_v<T, limited_int<std::int8_t>> ||
                           std::is_same_v<T, limited_int<std::int16_t>> ||
                           std::is_same_v<T, limited_int<std::int32_t>> ||
                           std::is_same_v<T, limited_int<std::int64_t>> ||
                           std::is_same_v<T, limited_int<std::uint8_t>> ||
                           std::is_same_v<T, limited_int<std::uint16_t>> ||
                           std::is_same_v<T, limited_int<std::uint32_t>> ||
                           std::is_same_v<T, limited_int<std::uint64_t>>)
        {
            using Int = typename T::value_type;
            Int value;
            BS_ASSERT(reader.read(value, arg.min, arg.max), "read #", read_item_index, " limited_int read failed");
            BS_ASSERT(value == arg.value, "read #", read_item_index, " limited_int expected = ", arg.value,
                      ", got = ", value, " for [", arg.min, ", ", arg.max, "]");
        }
        else if constexpr (std::is_same_v<T, char> || std::is_same_v<T, wchar_t> || std::is_same_v<T, char8_t> ||
                           std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>)
        {
            T ch;
            BS_ASSERT(reader.read(ch), "read #", read_item_index, "CharT read failed");
            BS_ASSERT(ch == arg, "read #", read_item_index, " CharT expected (0x", std::hex,
                      static_cast<std::uint32_t>(arg), "), got (0x", static_cast<std::uint32_t>(ch), ")", std::dec);
        }
        else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
        {
            T value;
            BS_ASSERT(reader.read(value), "read #", read_item_index, " float/double read failed");
            BS_ASSERT(value == arg, "read #", read_item_index, " float/double expected = ", arg, ", got = ", value);
        }
        else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring> ||
                           std::is_same_v<T, std::u8string> || std::is_same_v<T, std::u16string> ||
                           std::is_same_v<T, std::u32string>)
        {
            const auto len = reader.peek_string_length();
            BS_ASSERT(len > 0, "read #", read_item_index, " str length prefix read failed");

            T str;
            str.reserve(len);
            BS_ASSERT(reader.read(str, len), "read #", read_item_index, " str read failed");
            BS_ASSERT(str == arg, "read #", read_item_index, " str mismatch");
        }
        else
            static_assert(false, "Invalid input type");

        ++read_item_index;
        BS_ASSERT_READER_INVARIANTS;
    };

    // Read all the datas according to inputs
    for (const auto& input : g_inputs)
        std::visit(visitor, input);
}

} // namespace nalchi::tests

int main(int argc, char** argv)
{
    if (argc > 3)
    {
        std::cout << "=== Usage ===\n";
        std::cout << "`./test_bit_stream_stress`\n";
        std::cout << '\t' << "Runs the test 8 * " << BS_ITERATIONS << " times.\n";
        std::cout << "`./test_bit_stream_stress <iterations>`\n";
        std::cout << '\t' << "Runs the test 8 * <iterations> times.\n";
        std::cout << "`./test_bit_stream_stress <seed> <size>`";
        std::cout << '\t' << "Runs the test with specified <seed> and <size>.\n";
        return 2;
    }

    using namespace nalchi;
    using namespace nalchi::tests;

    std::cout << "=== bit_stream stress test ===\n";

    bit_stream_writer writer;
    bit_stream_reader reader;

    if (argc == 1 + 2)
    {
        seed_type seed = static_cast<seed_type>(std::atoll(argv[1]));
        size_type logical_bytes_length = static_cast<size_type>(std::atoll(argv[2]));

        if (logical_bytes_length > GNS_MAX_MSG_SEND_SIZE)
        {
            std::cout << "Specified size = " << logical_bytes_length << " is too big.\n";
            return 2;
        }

        std::cout << "Starting with seed = " << seed << ", size = " << logical_bytes_length << " ...\n";

        test_write_and_read(seed, logical_bytes_length, writer, reader);
    }
    else
    {
        std::size_t iterations = BS_ITERATIONS;

        if (argc == 1 + 1)
            iterations = static_cast<decltype(iterations)>(std::atoll(argv[1]));

        std::cout << "Starting 8 * " << iterations << " iterations...\n";

        rng_type rng(std::random_device{}());

        std::uniform_int_distribution<size_type> tiny(1, 16);
        std::uniform_int_distribution<size_type> small(17, 32);
        std::uniform_int_distribution<size_type> medium(33, 64);
        std::uniform_int_distribution<size_type> large(65, 128);
        std::uniform_int_distribution<size_type> extra(129, 256);
        std::uniform_int_distribution<size_type> extreme(257, 512);
        std::uniform_int_distribution<size_type> mtu(513, GNS_KNOWN_DEFAULT_MTU);
        std::uniform_int_distribution<size_type> fragmented(GNS_KNOWN_DEFAULT_MTU + 1, GNS_MAX_MSG_SEND_SIZE);

        for (std::size_t i = 0; i < iterations; ++i)
        {
            test_write_and_read(rng(), tiny(rng), writer, reader);
            test_write_and_read(rng(), small(rng), writer, reader);
            test_write_and_read(rng(), medium(rng), writer, reader);
            test_write_and_read(rng(), large(rng), writer, reader);
            test_write_and_read(rng(), extra(rng), writer, reader);
            test_write_and_read(rng(), extreme(rng), writer, reader);
            test_write_and_read(rng(), mtu(rng), writer, reader);
            test_write_and_read(rng(), fragmented(rng), writer, reader);
        }
    }

    std::cout << "bit_stream stress test succeeded" << std::endl;
}

#pragma once

#include "nalchi/export.hpp"

#include "nalchi/concepts.hpp"
#include "nalchi/shared_payload.hpp"
#include "nalchi/type_traits.hpp"

#include <algorithm>
#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#define NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(ret_val) \
    do \
    { \
        if (fail()) \
            return ret_val; \
    } while (false)

#define NALCHI_BIT_STREAM_WRITER_FAIL_IF_WRITE_AFTER_FINAL_FLUSH(ret_val) \
    do \
    { \
        if (_final_flushed) \
        { \
            _fail = true; \
            return ret_val; \
        } \
    } while (false)

#define NALCHI_BIT_STREAM_FAIL_IF_MIN_MAX_RANGE_INVALID(ret_val) \
    do \
    { \
        if (min >= max) \
        { \
            _fail = true; \
            return ret_val; \
        } \
    } while (false)

#define NALCHI_BIT_STREAM_WRITER_FAIL_IF_DATA_OUT_OF_RANGE(ret_val) \
    do \
    { \
        if (data < min || data > max) \
        { \
            _fail = true; \
            return ret_val; \
        } \
    } while (false)

#define NALCHI_BIT_STREAM_WRITER_FAIL_IF_STR_OVERFLOW(prefix_bytes, str_len_bytes) \
    do \
    { \
        if (_logical_used_bits + STR_LEN_PREFIX_PREFIX_BITS + (8 * (prefix_bytes)) + (8 * (str_len_bytes)) > \
            _logical_total_bits) \
        { \
            _fail = true; \
            return *this; \
        } \
    } while (false)

#define NALCHI_BIT_STREAM_READER_FAIL_IF_STR_OVERFLOW(str_len_bytes) \
    do \
    { \
        if (_logical_used_bits + (8 * (str_len_bytes)) > _logical_total_bits) \
        { \
            _fail = true; \
            return *this; \
        } \
    } while (false)

namespace nalchi
{

/// @brief Helper stream to write bits to your buffer.
///
/// Its design is based on the articles by Glenn Fiedler, see:
/// * https://gafferongames.com/post/reading_and_writing_packets/
/// * https://gafferongames.com/post/serialization_strategies/
///
/// @note `bit_stream_writer` uses an internal scratch buffer,
/// so the final few bytes might not be flushed to your buffer yet when you're done writing. \n
/// So, after writing everything, you @b must call `flush_final()` to flush the remaining bytes to your buffer. \n
/// (Destroying the `bit_stream_writer` instance won't flush them, either.)
class bit_stream_writer final
{
public:
    using size_type = std::uint32_t; ///< Size type representing number of bits and bytes.

    using scratch_type = std::uint64_t; ///< Internal scratch type to store the temporary scratch data.
    using word_type = std::uint32_t;    ///< Internal word type used to write to your buffer.

    static_assert(std::is_unsigned_v<scratch_type>);
    static_assert(std::is_unsigned_v<word_type>);
    static_assert(sizeof(scratch_type) == 2 * sizeof(word_type));

    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big,
                  "Mixed endian system is not supported");

public:
    // "prefix of string length prefix"
    // 0: u8 / 1: u16 / 2: u32 / 3: u64
    static constexpr size_type STR_LEN_PREFIX_PREFIX_BITS = 2u;
    static constexpr size_type MIN_STR_LEN_PREFIX_PREFIX = 0u;
    static constexpr size_type MAX_STR_LEN_PREFIX_PREFIX = 3u;

private:
    scratch_type _scratch;
    std::span<word_type> _words;

    int _scratch_index;
    int _words_index;

    // These fields are required, because logical user buffer size might differ from `_words` size.
    //
    // e.g. If user passed `shared_payload` whose size is 5 bytes,
    // the actual allocated buffer is 8 bytes, to avoid overrun writes.
    // But for the user's perspective, writing more than 5 bytes should be treated as an overflow.
    size_type _logical_total_bits;
    size_type _logical_used_bits;

    bool _init_fail;
    bool _fail;

    bool _final_flushed;

public:
    /// @brief Deleted copy constructor.
    bit_stream_writer(const bit_stream_writer&) = delete;

    /// @brief Deleted copy assignment operator.
    auto operator=(const bit_stream_writer&) -> bit_stream_writer& = delete;

    /// @brief Constructs a `bit_stream_writer` instance without a buffer.
    ///
    /// This constructor can be useful if you want to set the buffer afterwards. \n
    /// To set the buffer, call `reset_with()`.
    bit_stream_writer();

    /// @brief Constructs a `bit_stream_writer` instance with a `shared_payload` buffer.
    /// @param buffer Buffer to write bits to.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    bit_stream_writer(shared_payload buffer, size_type logical_bytes_length);

    /// @brief Constructs a `bit_stream_writer` instance with a `std::span<word_type>` buffer.
    /// @param buffer Buffer to write bits to.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    bit_stream_writer(std::span<word_type> buffer, size_type logical_bytes_length);

    /// @brief Constructs a `bit_stream_writer` instance with a word range.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param end Pointer to the end of a buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    bit_stream_writer(word_type* begin, word_type* end, size_type logical_bytes_length);

    /// @brief Constructs a `bit_stream_writer` instance with a word begin pointer and the word length.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param words_length Number of words in the buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    bit_stream_writer(word_type* begin, size_type words_length, size_type logical_bytes_length);

public:
    /// @brief Check if writing to your buffer has been failed or not.
    ///
    /// If this is `true`, all the operations for this `bit_stream_writer` is no-op.
    /// @return `true` if writing has been failed, otherwise `false`.
    bool fail() const noexcept
    {
        return _fail;
    }

    /// @brief Check if there was no error in the writing to your buffer. \n
    /// This is effectively same as `fail()`.
    ///
    /// If this is `true`, all the operations for this `bit_stream_writer` is no-op.
    bool operator!() const noexcept
    {
        return fail();
    }

    /// @brief Check if there was an error in the writing to your buffer. \n
    /// This is effectively same as `!fail()`.
    ///
    /// If this is `false`, all the operations for this `bit_stream_writer` is no-op.
    operator bool() const noexcept
    {
        return !fail();
    }

public:
    /// @brief Gets the number of total bytes in the stream.
    /// @return Number of total bytes in the stream.
    auto total_bytes() const -> size_type
    {
        return _logical_total_bits / 8;
    }

    /// @brief Gets the number of total bits in the stream.
    /// @return Number of total bits in the stream.
    auto total_bits() const -> size_type
    {
        return _logical_total_bits;
    }

    /// @brief Gets the number of used bytes in the stream.
    /// @return Number of used bytes in the stream.
    auto used_bytes() const -> size_type;

    /// @brief Gets the number of used bits in the stream.
    /// @return Number of used bits in the stream.
    auto used_bits() const -> size_type
    {
        return _logical_used_bits;
    }

    /// @brief Gets the number of unused bytes in the stream.
    /// @return Number of unused bytes in the stream.
    auto unused_bytes() const -> size_type
    {
        return total_bytes() - used_bytes();
    }

    /// @brief Gets the number of unused bits in the stream.
    /// @return Number of unused bits in the stream.
    auto unused_bits() const -> size_type
    {
        return total_bits() - used_bits();
    }

public:
    /// @brief Restarts the stream so that it can write from the beginning again.
    /// @note This function resets internal states @b without flushing,
    /// so if you need flushing, you should call `flush_final()` beforehand.
    void restart();

    /// @brief Resets the stream so that it no longer holds your buffer anymore.
    /// @note This function removes reference to your buffer @b without flushing, \n
    /// so if you need flushing, you should call `flush_final()` beforehand.
    void reset();

    /// @brief Resets the stream with a `shared_payload` buffer.
    /// @note This function resets to the new buffer @b without flushing to your previous buffer, \n
    /// so if you need flushing, you should call `flush_final()` beforehand.
    /// @param buffer Buffer to write bits to.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    void reset_with(shared_payload buffer, size_type logical_bytes_length);

    /// @brief Resets the stream with a `std::span<word_type>` buffer.
    /// @note This function resets to the new buffer @b without flushing to your previous buffer, \n
    /// so if you need flushing, you should call `flush_final()` beforehand.
    /// @param buffer Buffer to write bits to.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    void reset_with(std::span<word_type> buffer, size_type logical_bytes_length);

    /// @brief Resets the stream with a word range.
    /// @note This function resets to the new buffer @b without flushing to your previous buffer, \n
    /// so if you need flushing, you should call `flush_final()` beforehand.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param end Pointer to the end of a buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    void reset_with(word_type* begin, word_type* end, size_type logical_bytes_length);

    /// @brief Resets the stream with a word begin pointer and the word length.
    /// @note This function resets to the new buffer @b without flushing to your previous buffer, \n
    /// so if you need flushing, you should call `flush_final()` beforehand.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param words_length Number of words in the buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    void reset_with(word_type* begin, size_type words_length, size_type logical_bytes_length);

    /// @brief Flushes the last remaining bytes on the internal scratch buffer to your buffer.
    /// @note This function must be only called when you're done writing. \n
    /// Any attempt to write more data after calling this function will set the fail flag and write nothing.
    /// @return The stream itself.
    auto flush_final() -> bit_stream_writer&;

    /// @brief Checks if `flush_final()` has been called or not.
    /// @return Whether the `flush_final()` has been called or not.
    bool flushed() const
    {
        return _final_flushed;
    }

public:
    /// @brief Writes some arbitrary data to the bit stream.
    /// @note Bytes in your data could be read @b swapped if it is sent to the system with different endianness. \n
    /// So, prefer using other overloads instead.
    /// @param data Pointer to the arbitrary data.
    /// @param size Size in bytes of the data.
    /// @return The stream itself.
    auto write(const void* data, size_type size) -> bit_stream_writer&;

    /// @brief Writes an integral value to the bit stream.
    /// @tparam SInt Small integer type that doesn't exceed the size of `word_type`.
    /// @param data Data to write.
    /// @param min Minimum value allowed for @p data.
    /// @param max Maximum value allowed for @p data.
    /// @return The stream itself.
    template <std::integral SInt>
        requires(sizeof(SInt) <= sizeof(word_type))
    auto write(SInt data, SInt min = std::numeric_limits<SInt>::min(), SInt max = std::numeric_limits<SInt>::max())
        -> bit_stream_writer&
    {
        return do_write<true>(data, min, max);
    }

    /// @brief Writes an integral value to the bit stream.
    /// @tparam BInt Big integer type that exceeds the size of `word_type`.
    /// @param data Data to write.
    /// @param min Minimum value allowed for @p data.
    /// @param max Maximum value allowed for @p data.
    /// @return The stream itself.
    template <std::integral BInt>
        requires(sizeof(BInt) > sizeof(word_type))
    auto write(BInt data, BInt min = std::numeric_limits<BInt>::min(), BInt max = std::numeric_limits<BInt>::max())
        -> bit_stream_writer&
    {
        return do_write<true>(data, min, max);
    }

    /// @brief Writes a float value to the bit stream.
    /// @param data Data to write.
    /// @return The stream itself.
    auto write(float data) -> bit_stream_writer&;

    /// @brief Writes a double value to the bit stream.
    /// @param data Data to write.
    /// @return The stream itself.
    auto write(double data) -> bit_stream_writer&;

    /// @brief Writes a string view to the bit stream.
    /// @tparam CharT Underlying character type of `std::basic_string_view`.
    /// @tparam CharTraits Char traits for `CharT`.
    /// @param str String to write.
    /// @return The stream itself.
    template <character CharT, typename CharTraits>
    auto write(std::basic_string_view<CharT, CharTraits> str) -> bit_stream_writer&
    {
        NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);
        NALCHI_BIT_STREAM_WRITER_FAIL_IF_WRITE_AFTER_FINAL_FLUSH(*this);

        // Get the length of the string.
        const auto len = str.length();

        // Write a "prefix of length prefix" + length prefix.
        // 0: u8 / 1: u16 / 2: u32 / 3: u64
        if (len <= std::numeric_limits<std::uint8_t>::max())
        {
            NALCHI_BIT_STREAM_WRITER_FAIL_IF_STR_OVERFLOW(sizeof(std::uint8_t), len * sizeof(CharT));
            do_write<false>(size_type(0), MIN_STR_LEN_PREFIX_PREFIX,
                            MAX_STR_LEN_PREFIX_PREFIX);      // prefix of length prefix
            do_write<false>(static_cast<std::uint8_t>(len)); // length prefix
        }
        else if (len <= std::numeric_limits<std::uint16_t>::max())
        {
            NALCHI_BIT_STREAM_WRITER_FAIL_IF_STR_OVERFLOW(sizeof(std::uint16_t), len * sizeof(CharT));
            do_write<false>(size_type(1), MIN_STR_LEN_PREFIX_PREFIX,
                            MAX_STR_LEN_PREFIX_PREFIX);       // prefix of length prefix
            do_write<false>(static_cast<std::uint16_t>(len)); // length prefix
        }
        else if (len <= std::numeric_limits<std::uint32_t>::max())
        {
            NALCHI_BIT_STREAM_WRITER_FAIL_IF_STR_OVERFLOW(sizeof(std::uint32_t), len * sizeof(CharT));
            do_write<false>(size_type(2), MIN_STR_LEN_PREFIX_PREFIX,
                            MAX_STR_LEN_PREFIX_PREFIX);       // prefix of length prefix
            do_write<false>(static_cast<std::uint32_t>(len)); // length prefix
        }
        else // len <= std::numeric_limits<std::uint64_t>::max()
        {
            NALCHI_BIT_STREAM_WRITER_FAIL_IF_STR_OVERFLOW(sizeof(std::uint64_t), len * sizeof(CharT));
            do_write<false>(size_type(3), MIN_STR_LEN_PREFIX_PREFIX,
                            MAX_STR_LEN_PREFIX_PREFIX);       // prefix of length prefix
            do_write<false>(static_cast<std::uint64_t>(len)); // length prefix
        }

        // Just write every character one by one.
        for (const auto ch : str)
            do_write<false>(ch);

        return *this;
    }

    /// @brief Writes a string to the bit stream.
    /// @tparam CharT Underlying character type of `std::basic_string`.
    /// @tparam CharTraits Char traits for `CharT`.
    /// @tparam Allocator Underlying allocator for `std::basic_string`.
    /// @param str String to write.
    /// @return The stream itself.
    template <character CharT, typename CharTraits, typename Allocator>
    auto write(const std::basic_string<CharT, CharTraits, Allocator>& str) -> bit_stream_writer&
    {
        return write(std::basic_string_view<CharT, CharTraits>(str));
    }

    /// @brief Writes a null-terminated string to the bit stream.
    /// @tparam CharT Character type of the null-terminated string.
    /// @param str String to write.
    /// @return The stream itself.
    template <character CharT>
    auto write(const CharT* str) -> bit_stream_writer&
    {
        return write(std::basic_string_view<CharT>(str));
    }

private:
    /// @brief Actually writes an integral value to the bit stream.
    /// @tparam Checked Whether the checks are performed or not.
    /// @tparam SInt Small integer type that doesn't exceed the size of `word_type`.
    /// @param data Data to write.
    /// @param min Minimum value allowed for @p data.
    /// @param max Maximum value allowed for @p data.
    /// @return The stream itself.
    template <bool Checked, std::integral SInt>
        requires(sizeof(SInt) <= sizeof(word_type))
    auto do_write(SInt data, SInt min = std::numeric_limits<SInt>::min(), SInt max = std::numeric_limits<SInt>::max())
        -> bit_stream_writer&
    {
        if constexpr (Checked)
        {
            NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);
            NALCHI_BIT_STREAM_WRITER_FAIL_IF_WRITE_AFTER_FINAL_FLUSH(*this);
            NALCHI_BIT_STREAM_FAIL_IF_MIN_MAX_RANGE_INVALID(*this);
            NALCHI_BIT_STREAM_WRITER_FAIL_IF_DATA_OUT_OF_RANGE(*this);
        }

        using UInt = make_unsigned_allow_bool_t<SInt>;

        // Convert `data` to `value` to actually write.
        const scratch_type value = static_cast<UInt>(((UInt)data) - ((UInt)min));
        const int bits = std::bit_width(static_cast<UInt>(((UInt)max) - ((UInt)min)));

        if constexpr (Checked)
        {
            // Fail if user buffer overflows.
            if (_logical_used_bits + bits > _logical_total_bits)
            {
                _fail = true;
                return *this;
            }
        }

        // Write `value` to `_scratch`, and flush if scratch overflow.
        _scratch |= (value << _scratch_index);
        _scratch_index += bits;
        flush_if_scratch_overflow();

        // Adjust used bits
        _logical_used_bits += bits;

        return *this;
    }

    /// @brief Actually writes an integral value to the bit stream.
    /// @tparam Checked Whether the checks are performed or not.
    /// @tparam BInt Big integer type that exceeds the size of `word_type`.
    /// @param data Data to write.
    /// @param min Minimum value allowed for @p data.
    /// @param max Maximum value allowed for @p data.
    /// @return The stream itself.
    template <bool Checked, std::integral BInt>
        requires(sizeof(BInt) > sizeof(word_type))
    auto do_write(BInt data, BInt min = std::numeric_limits<BInt>::min(), BInt max = std::numeric_limits<BInt>::max())
        -> bit_stream_writer&
    {
        if constexpr (Checked)
        {
            NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);
            NALCHI_BIT_STREAM_WRITER_FAIL_IF_WRITE_AFTER_FINAL_FLUSH(*this);
            NALCHI_BIT_STREAM_FAIL_IF_MIN_MAX_RANGE_INVALID(*this);
            NALCHI_BIT_STREAM_WRITER_FAIL_IF_DATA_OUT_OF_RANGE(*this);
        }

        // Current logic assumes the only size here is 64 bits, but hey who wouldn't?
        static_assert(sizeof(BInt) == 2 * sizeof(word_type));

        using UInt = make_unsigned_allow_bool_t<BInt>;

        // Convert `data` to `value`.
        const scratch_type value = static_cast<UInt>(((UInt)data) - ((UInt)min));
        const int bits = std::bit_width(static_cast<UInt>(((UInt)max) - ((UInt)min)));

        if constexpr (Checked)
        {
            // Fail if user buffer overflows.
            if (_logical_used_bits + bits > _logical_total_bits)
            {
                _fail = true;
                return *this;
            }
        }

        // Split lower half of `value`
        const scratch_type low = (value << (8 * sizeof(word_type))) >> (8 * sizeof(word_type));
        const int low_bits = std::min(bits, static_cast<int>(8 * sizeof(word_type)));

        // Write lower half to `_scratch`, and flush if scratch overflow.
        _scratch |= (low << _scratch_index);
        _scratch_index += low_bits;
        flush_if_scratch_overflow();

        const int high_bits = bits - low_bits;
        if (high_bits > 0)
        {
            // Split higher half of `value`
            const scratch_type high = (value >> (8 * sizeof(word_type)));

            // Write higher half to `_scratch`, and flush if scratch overflow.
            _scratch |= (high << _scratch_index);
            _scratch_index += high_bits;
            flush_if_scratch_overflow();
        }

        // Adjust used bits
        _logical_used_bits += bits;

        return *this;
    }

private:
    void flush_if_scratch_overflow();

    /// @brief Actually flushes from the internal scratch buffer to the user buffer.
    /// @note This function flushes the internal scratch word as-is, \n
    /// which means calling this mid-way through writing can
    /// write some undesired additional `0` bits in the middle of your buffer. \n
    /// To avoid that, you should only call this when you're done writing everything.
    /// @return The stream itself.
    void do_flush_word_unchecked();
};

/// @brief Helper stream to read bits from your buffer.
///
/// Its design is based on the articles by Glenn Fiedler, see:
/// * https://gafferongames.com/post/reading_and_writing_packets/
/// * https://gafferongames.com/post/serialization_strategies/
class bit_stream_reader final
{
public:
    using size_type = bit_stream_writer::size_type;   ///< Size type representing number of bits and bytes.
    using ssize_type = std::make_signed_t<size_type>; ///< Signed size type to allow negative error value.

    using scratch_type =
        bit_stream_writer::scratch_type;            ///< Internal scratch type to store the temporary scratch data.
    using word_type = bit_stream_writer::word_type; ///< Internal word type used to read from your buffer.

private:
    scratch_type _scratch;
    std::span<const word_type> _words;

    int _scratch_bits;
    int _words_index;

    size_type _logical_total_bits;
    size_type _logical_used_bits;

    bool _init_fail;
    bool _fail;

public:
    /// @brief Deleted copy constructor.
    bit_stream_reader(const bit_stream_reader&) = delete;

    /// @brief Deleted copy assignment operator.
    auto operator=(const bit_stream_reader&) -> bit_stream_reader& = delete;

    /// @brief Constructs a `bit_stream_reader` instance without a buffer.
    ///
    /// This constructor can be useful if you want to set the buffer afterwards. \n
    /// To set the buffer, call `reset_with()`.
    bit_stream_reader();

    /// @brief Constructs a `bit_stream_reader` instance with a `std::span<word_type>` buffer.
    /// @param buffer Buffer to read bits from.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial read from the final word.
    bit_stream_reader(std::span<const word_type> buffer, size_type logical_bytes_length);

    /// @brief Constructs a `bit_stream_reader` instance with a word range.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param end Pointer to the end of a buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial read from the final word.
    bit_stream_reader(const word_type* begin, const word_type* end, size_type logical_bytes_length);

    /// @brief Constructs a `bit_stream_reader` instance with a word begin pointer and the word length.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param words_length Number of words in the buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial read from the final word.
    bit_stream_reader(const word_type* begin, size_type words_length, size_type logical_bytes_length);

public:
    /// @brief Check if reading from your buffer has been failed or not.
    ///
    /// If this is `true`, all the operations for this `bit_stream_reader` is no-op.
    /// @return `true` if reading has been failed, otherwise `false`.
    bool fail() const noexcept
    {
        return _fail;
    }

    /// @brief Check if there was no error in the reading to your buffer. \n
    /// This is effectively same as `fail()`.
    ///
    /// If this is `true`, all the operations for this `bit_stream_reader` is no-op.
    bool operator!() const noexcept
    {
        return fail();
    }

    /// @brief Check if there was an error in the reading to your buffer. \n
    /// This is effectively same as `!fail()`.
    ///
    /// If this is `false`, all the operations for this `bit_stream_reader` is no-op.
    operator bool() const noexcept
    {
        return !fail();
    }

public:
    /// @brief Restarts the stream so that it can read from the beginning again.
    void restart();

    /// @brief Resets the stream so that it no longer holds your buffer anymore.
    void reset();

    /// @brief Resets the stream with a `std::span<word_type>` buffer.
    /// @param buffer Buffer to read bits from.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial read from the final word.
    void reset_with(std::span<const word_type> buffer, size_type logical_bytes_length);

    /// @brief Resets the stream with a word range.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param end Pointer to the end of a buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial read from the final word.
    void reset_with(const word_type* begin, const word_type* end, size_type logical_bytes_length);

    /// @brief Resets the stream with a word begin pointer and the word length.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param words_length Number of words in the buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial read from the final word.
    void reset_with(const word_type* begin, size_type words_length, size_type logical_bytes_length);

public:
    /// @brief Reads some arbitrary data from the bit stream.
    /// @note You could read @b swapped bytes if the data came from the system with different endianness. \n
    /// So, prefer using other overloads instead.
    /// @param data Pointer to the arbitrary data.
    /// @param size Size in bytes of the data.
    /// @return The stream itself.
    auto read(void* data, size_type size) -> bit_stream_reader&;

    /// @brief Reads an integral value from the bit stream.
    /// @tparam SInt Small integer type that doesn't exceed the size of `word_type`.
    /// @param data Data to read to.
    /// @param min Minimum value allowed for @p data.
    /// @param max Maximum value allowed for @p data.
    /// @return The stream itself.
    template <std::integral SInt>
        requires(sizeof(SInt) <= sizeof(word_type))
    auto read(SInt& data, SInt min = std::numeric_limits<SInt>::min(), SInt max = std::numeric_limits<SInt>::max())
        -> bit_stream_reader&
    {
        return do_read<true>(data, min, max);
    }

    /// @brief Reads an integral value from the bit stream.
    /// @tparam BInt Big integer type that exceeds the size of `word_type`.
    /// @param data Data to read to.
    /// @param min Minimum value allowed for @p data.
    /// @param max Maximum value allowed for @p data.
    /// @return The stream itself.
    template <std::integral BInt>
        requires(sizeof(BInt) > sizeof(word_type))
    auto read(BInt& data, BInt min = std::numeric_limits<BInt>::min(), BInt max = std::numeric_limits<BInt>::max())
        -> bit_stream_reader&
    {
        return do_read<true>(data, min, max);
    }

    /// @brief Reads a float value from the bit stream.
    /// @param data Data to read to.
    /// @return The stream itself.
    auto read(float& data) -> bit_stream_reader&;

    /// @brief Reads a double value from the bit stream.
    /// @param data Data to read to.
    /// @return The stream itself.
    auto read(double& data) -> bit_stream_reader&;

    /// @brief Reads a string from the bit stream.
    ///
    /// If the length prefix for current stream position exceeds @p max_length, \n
    /// this function will set the fail flag and read nothing.
    /// @tparam CharT Underlying character type of `std::basic_string`.
    /// @tparam CharTraits Char traits for `CharT`.
    /// @tparam Allocator Underlying allocator for `std::basic_string`.
    /// @param str String to read to.
    /// @param max_length Maximum number of `CharT` that can be read. \n
    /// This is to prevent a huge allocation when a malicious message requests it.
    /// @return The stream itself.
    template <character CharT, typename CharTraits, typename Allocator>
    auto read(std::basic_string<CharT, CharTraits, Allocator>& str, size_type max_length) -> bit_stream_reader&
    {
        NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);

        // Read the length of the string.
        ssize_type len = read_string_length();
        if (len < 0 || len > max_length)
        {
            _fail = true;
            return *this;
        }

        NALCHI_BIT_STREAM_READER_FAIL_IF_STR_OVERFLOW(len * sizeof(CharT));

        // Resize and read every character one by one.
        str.resize(len);
        for (auto& ch : str)
            do_read<false>(ch);

        return *this;
    }

    /// @brief Reads a null-terminated string from the bit stream.
    ///
    /// If @p max_length is not enough to store the string, \n
    /// this function will set the fail flag and read nothing.
    /// @note @p max_length does @b not include null character, so your buffer @b must allocate additional space for it.
    ///
    /// For example, if @p max_length is 4 for `char16_t`, you need 10 bytes. \n
    /// Because you need space for 5 `char16_t` including null char, and `char16_t` is 2 bytes per char.
    /// @tparam CharT Character type of the null-terminated string.
    /// @param str Null-terminated string to read to.
    /// @param max_length Maximum number of `CharT` that can be read.
    /// @return The stream itself.
    template <character CharT>
    auto read(CharT* str, size_type max_length) -> bit_stream_reader&
    {
        NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);

        // Read the length of the string.
        ssize_type len = read_string_length();
        if (len < 0 || len > max_length)
        {
            _fail = true;
            return *this;
        }

        NALCHI_BIT_STREAM_READER_FAIL_IF_STR_OVERFLOW(len * sizeof(CharT));

        // Read every character one by one.
        for (ssize_type i = 0; i < len; ++i)
            do_read<false>(str[i]);

        // Insert final null character.
        str[len] = CharT(0);

        return *this;
    }

    /// @brief Peeks the string length prefix from the current stream position.
    ///
    /// If it fails to read a string length prefix, \n
    /// this function will return a negative value and set the fail flag.
    /// @note Be careful, if current stream position was not on the string length prefix, it might read garbage length!
    /// @return Length of `CharT` stored in it, or a negative value if length prefix is invalid.
    auto peek_string_length() -> ssize_type;

private:
    /// @brief Reads the string length prefix from the current stream position.
    ///
    /// If it fails to read a string length prefix, \n
    /// this function will return a negative value and set the fail flag.
    /// @note Be careful, if current stream position was not on the string length prefix, it might read garbage length!
    /// @return Length of `CharT` stored in it, or a negative value if length prefix is invalid.
    auto read_string_length() -> ssize_type;

    /// @brief Actually reads an integral value from the bit stream.
    /// @tparam Checked Whether the checks are performed or not.
    /// @tparam SInt Small integer type that doesn't exceed the size of `word_type`.
    /// @param data Data to read to.
    /// @param min Minimum value allowed for @p data.
    /// @param max Maximum value allowed for @p data.
    /// @return The stream itself.
    template <bool Checked, std::integral SInt>
        requires(sizeof(SInt) <= sizeof(word_type))
    auto do_read(SInt& data, SInt min = std::numeric_limits<SInt>::min(), SInt max = std::numeric_limits<SInt>::max())
        -> bit_stream_reader&
    {
        if constexpr (Checked)
        {
            NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);
            NALCHI_BIT_STREAM_FAIL_IF_MIN_MAX_RANGE_INVALID(*this);
        }

        using UInt = make_unsigned_allow_bool_t<SInt>;

        // Calculate bits to read.
        const int bits = std::bit_width(static_cast<UInt>(((UInt)max) - ((UInt)min)));

        if constexpr (Checked)
        {
            // Fail if no more data to be read in `_words`.
            if (_logical_used_bits + bits > _logical_total_bits)
            {
                _fail = true;
                return *this;
            }
        }

        // Load more bits to `_scratch` if needed.
        if (bits > _scratch_bits)
            do_fetch_word_unchecked();

        // Read raw `value` from `_scratch`.
        UInt value = static_cast<UInt>(_scratch & ((((scratch_type)1) << bits) - 1));

        // Remove read bits from `_scratch`.
        _scratch >>= bits;
        _scratch_bits -= bits;

        // Convert to original range.
        const SInt conv = static_cast<SInt>(((SInt)value) + min);

        if constexpr (Checked)
        {
            // Fail if it exceeds `max`.
            if (conv > max)
            {
                _fail = true;
                return *this;
            }
        }

        // Load `conv` to `data`.
        data = conv;

        // Adjust used bits
        _logical_used_bits += bits;

        return *this;
    }

    /// @brief Actually reads an integral value from the bit stream.
    /// @tparam Checked Whether the checks are performed or not.
    /// @tparam BInt Big integer type that exceeds the size of `word_type`.
    /// @param data Data to read to.
    /// @param min Minimum value allowed for @p data.
    /// @param max Maximum value allowed for @p data.
    /// @return The stream itself.
    template <bool Checked, std::integral BInt>
        requires(sizeof(BInt) > sizeof(word_type))
    auto do_read(BInt& data, BInt min = std::numeric_limits<BInt>::min(), BInt max = std::numeric_limits<BInt>::max())
        -> bit_stream_reader&
    {
        if constexpr (Checked)
        {
            NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);
            NALCHI_BIT_STREAM_FAIL_IF_MIN_MAX_RANGE_INVALID(*this);
        }

        // Current logic assumes the only size here is 64 bits, but hey who wouldn't?
        static_assert(sizeof(BInt) == 2 * sizeof(word_type));

        using UInt = make_unsigned_allow_bool_t<BInt>;

        // Calculate bits to read.
        const int bits = std::bit_width(static_cast<UInt>(((UInt)max) - ((UInt)min)));

        if constexpr (Checked)
        {
            // Fail if no more data to be read in `_words`.
            if (_logical_used_bits + bits > _logical_total_bits)
            {
                _fail = true;
                return *this;
            }
        }

        const int low_bits = std::min(bits, static_cast<int>(8 * sizeof(word_type)));

        // Load more bits to `_scratch` if needed.
        if (low_bits > _scratch_bits)
            do_fetch_word_unchecked();

        // Read low bits from `_scratch`.
        UInt value = static_cast<UInt>(_scratch & ((((scratch_type)1) << low_bits) - 1));

        // Remove read bits from `_scratch`.
        _scratch >>= low_bits;
        _scratch_bits -= low_bits;

        const int high_bits = bits - low_bits;
        if (high_bits > 0)
        {
            // Load more bits to `_scratch` if needed.
            if (high_bits > _scratch_bits)
                do_fetch_word_unchecked();

            // Read high bits from `_scratch`.
            value |= (static_cast<UInt>(_scratch & ((((scratch_type)1) << high_bits) - 1)) << low_bits);

            // Remove read bits from `_scratch`.
            _scratch >>= high_bits;
            _scratch_bits -= high_bits;
        }

        // Convert to original range.
        const BInt conv = static_cast<BInt>(((BInt)value) + min);

        if constexpr (Checked)
        {
            // Fail if it exceeds `max`.
            if (conv > max)
            {
                _fail = true;
                return *this;
            }
        }

        // Load `conv` to `data`.
        data = conv;

        // Adjust used bits
        _logical_used_bits += bits;

        return *this;
    }

private:
    void do_fetch_word_unchecked();
};

} // namespace nalchi

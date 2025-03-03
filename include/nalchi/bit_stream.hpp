#pragma once

#include "nalchi/export.h"

#include <algorithm>
#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>
#include <span>
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
        if (_final_flush) \
        { \
            _fail = true; \
            return ret_val; \
        } \
    } while (false)

#define NALCHI_BIT_STREAM_WRITER_FAIL_IF_DATA_OUT_OF_RANGE(ret_val) \
    do \
    { \
        if (min >= max || data < min || data > max) \
        { \
            _fail = true; \
            return ret_val; \
        } \
    } while (false)

struct nalchi_payload;

namespace nalchi
{

/// @brief Bit stream writer to help writing bits to your buffer.
///
/// Its design is based on the articles by Glenn Fiedler, see:
/// * https://gafferongames.com/post/reading_and_writing_packets/
/// * https://gafferongames.com/post/serialization_strategies/
///
/// @note `bit_stream_writer` uses an internal scratch buffer,
/// so the final few bytes might not be flushed to your buffer yet when you're done writing. \n
/// To flush them, you can call `flush_final()` directly,
/// or destroy the `bit_stream_writer` instance.
class bit_stream_writer final
{
public:
    using scratch_type = std::uint64_t; ///< Internal scratch type to store the temporary scratch data.
    using word_type = std::uint32_t;    ///< Internal word type used to write to your buffer.

    static_assert(std::is_unsigned_v<scratch_type>);
    static_assert(std::is_unsigned_v<word_type>);
    static_assert(sizeof(scratch_type) == 2 * sizeof(word_type));

    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big,
                  "Mixed endian system is not supported");

private:
    scratch_type _scratch;
    const std::span<word_type> _words;

    int _scratch_index;
    int _words_index;

    // These fields are required, because logical user buffer size might differ from `_words` size.
    //
    // e.g. If user passed `nalchi_payload` whose size is 5 bytes,
    // the actual allocated buffer is 8 bytes, to avoid overrun writes.
    // But for the user's perspective, writing more than 5 bytes should be treated as an overflow.
    const std::int64_t _logical_total_bits;
    std::int64_t _logical_used_bits;

    const bool _init_fail;
    bool _fail;

    bool _final_flush;

public:
    /// @brief Deleted copy constructor.
    bit_stream_writer(const bit_stream_writer&) = delete;

    /// @brief Constructs a `bit_stream_writer` instance with a `nalchi_payload` buffer.
    /// @param buffer Buffer to write bits to.
    bit_stream_writer(nalchi_payload buffer);

    /// @brief Constructs a `bit_stream_writer` instance with a `std::span<word_type>` buffer.
    /// @param buffer Buffer to write bits to.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    bit_stream_writer(std::span<word_type> buffer, int logical_bytes_length);

    /// @brief Constructs a `bit_stream_writer` instance with a word range.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param end Pointer to the end of a buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    bit_stream_writer(word_type* begin, word_type* end, int logical_bytes_length);

    /// @brief Constructs a `bit_stream_writer` instance with a word begin pointer and the word length.
    /// @param begin Pointer to the beginning of a buffer.
    /// @param words_length Number of words in the buffer.
    /// @param logical_bytes_length Number of bytes logically.
    /// This is useful if you want to only allow partial write to the final word.
    bit_stream_writer(word_type* begin, int words_length, int logical_bytes_length);

    /// @brief Destroys the `bit_stream_writer` instance.
    ~bit_stream_writer();

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
    auto total_bytes() const -> std::int64_t
    {
        return _logical_total_bits / 8;
    }

    /// @brief Gets the number of total bits in the stream.
    /// @return Number of total bits in the stream.
    auto total_bits() const -> std::int64_t
    {
        return _logical_total_bits;
    }

    /// @brief Gets the number of used bytes in the stream.
    /// @return Number of used bytes in the stream.
    auto used_bytes() const -> std::int64_t;

    /// @brief Gets the number of used bits in the stream.
    /// @return Number of used bits in the stream.
    auto used_bits() const -> std::int64_t
    {
        return _logical_used_bits;
    }

    /// @brief Gets the number of unused bytes in the stream.
    /// @return Number of unused bytes in the stream.
    auto unused_bytes() const -> std::int64_t
    {
        return total_bytes() - used_bytes();
    }

    /// @brief Gets the number of unused bits in the stream.
    /// @return Number of unused bits in the stream.
    auto unused_bits() const -> std::int64_t
    {
        return total_bits() - used_bits();
    }

public:
    /// @brief Resets the stream so that it can write from the start.
    void reset();

    /// @brief Flushes the last remaining bytes on the internal scratch buffer to your buffer.
    /// @note This function must be only called when you're done writing. \n
    /// Any attempt to write more data after calling this function will set the fail flag and write nothing.
    /// @return The stream itself.
    auto flush_final() -> bit_stream_writer&;

public:
    /// @brief Writes a numeric value to the bit stream.
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
        NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);
        NALCHI_BIT_STREAM_WRITER_FAIL_IF_WRITE_AFTER_FINAL_FLUSH(*this);
        NALCHI_BIT_STREAM_WRITER_FAIL_IF_DATA_OUT_OF_RANGE(*this);

        using UInt = std::make_unsigned_t<SInt>;

        // Convert `data` to `value` to actually write.
        const scratch_type value = ((UInt)data) - ((UInt)min);
        const int bits = std::bit_width(value);

        // Fail if user buffer overflows.
        if (_logical_used_bits + bits > _logical_total_bits)
        {
            _fail = true;
            return *this;
        }

        // Write `value` to `_scratch`, and flush if scratch overflow.
        _scratch |= (value << _scratch_index);
        _scratch_index += bits;
        flush_if_scratch_overflow();

        // Adjust used bits
        _logical_used_bits += bits;

        return *this;
    }

    /// @brief Writes a numeric value to the bit stream.
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
        NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);
        NALCHI_BIT_STREAM_WRITER_FAIL_IF_WRITE_AFTER_FINAL_FLUSH(*this);
        NALCHI_BIT_STREAM_WRITER_FAIL_IF_DATA_OUT_OF_RANGE(*this);

        // Current logic assumes the only size here is 64 bits, but hey who wouldn't?
        static_assert(sizeof(BInt) == 2 * sizeof(word_type));

        using UInt = std::make_unsigned_t<BInt>;

        // Convert `data` to `value`.
        const scratch_type value = ((UInt)data) - ((UInt)min);
        const int bits = std::bit_width(value);

        // Fail if user buffer overflows.
        if (_logical_used_bits + bits > _logical_total_bits)
        {
            _fail = true;
            return *this;
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

    /// @brief Writes a float value to the bit stream.
    /// @param data Data to write.
    /// @return The stream itself.
    auto write(float data) -> bit_stream_writer&;

    /// @brief Writes a double value to the bit stream.
    /// @param data Data to write.
    /// @return The stream itself.
    auto write(double data) -> bit_stream_writer&;

private:
    void flush_if_scratch_overflow();

    /// @brief Actually flushes from the internal scratch buffer to the user buffer.
    /// @note This function flushes the internal scratch word as-is, \n
    /// which means calling this mid-way through writing can
    /// write some undesired additional `0` bits in the middle of your buffer. \n
    /// To avoid that, you should only call this when you're done writing everything.
    /// @return The stream itself.
    void flush_word_unchecked();
};

} // namespace nalchi

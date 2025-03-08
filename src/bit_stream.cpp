#include "nalchi/bit_stream.hpp"

#include "nalchi/shared_payload.hpp"

#include "math.hpp"

#include <steam/steamnetworkingtypes.h>

#include <algorithm>
#include <cstddef>
#include <utility>

namespace nalchi
{

namespace
{

constexpr auto GNS_MAX_MSG_RECV_SIZE = 2 * k_cbMaxSteamNetworkingSocketsMessageSizeSend;
static_assert(8 * GNS_MAX_MSG_RECV_SIZE <= std::numeric_limits<bit_stream_writer::size_type>::max(),
              "`bit_stream_writer::size_type` too small to represent incoming GNS message in number of bits");

} // namespace

bit_stream_writer::bit_stream_writer()
{
    reset();
}

bit_stream_writer::bit_stream_writer(shared_payload buffer, size_type logical_bytes_length)
{
    reset_with(buffer, logical_bytes_length);
}

bit_stream_writer::bit_stream_writer(std::span<word_type> buffer, size_type logical_bytes_length)
{
    reset_with(buffer, logical_bytes_length);
}

bit_stream_writer::bit_stream_writer(word_type* begin, word_type* end, size_type logical_bytes_length)
{
    reset_with(begin, end, logical_bytes_length);
}

bit_stream_writer::bit_stream_writer(word_type* begin, size_type words_length, size_type logical_bytes_length)
{
    reset_with(begin, words_length, logical_bytes_length);
}

auto bit_stream_writer::used_bytes() const -> size_type
{
    return ceil_to_multiple_of<8>(used_bits()) / 8;
}

void bit_stream_writer::restart()
{
    _scratch = 0;

    _scratch_index = 0;
    _words_index = 0;

    _logical_used_bits = 0;
    _fail = _init_fail;

    _final_flushed = false;
}

void bit_stream_writer::reset()
{
    _words = decltype(_words)();
    _logical_total_bits = 0;
    _init_fail = true;

    restart();
}

void bit_stream_writer::reset_with(shared_payload buffer, size_type logical_bytes_length)
{
    reset_with(
        std::span<word_type>(reinterpret_cast<word_type*>(buffer.ptr), buffer.get_aligned_size() / sizeof(word_type)),
        logical_bytes_length);
}

void bit_stream_writer::reset_with(std::span<word_type> buffer, size_type logical_bytes_length)
{
    _words = buffer;
    _logical_total_bits = 8 * logical_bytes_length;
    _init_fail = (!buffer.data() || buffer.size() == 0 || std::size_t(logical_bytes_length) > buffer.size_bytes());

    restart();
}

void bit_stream_writer::reset_with(word_type* begin, word_type* end, size_type logical_bytes_length)
{
    reset_with(std::span<word_type>(begin, end), logical_bytes_length);
}

void bit_stream_writer::reset_with(word_type* begin, size_type words_length, size_type logical_bytes_length)
{
    reset_with(std::span<word_type>(begin, words_length), logical_bytes_length);
}

auto bit_stream_writer::flush_final() -> bit_stream_writer&
{
    NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);

    // No-op if already flushed
    if (_final_flushed)
        return *this;

    // No-op if nothing to flush
    if (_scratch_index > 0)
        do_flush_word_unchecked();

    _final_flushed = true;

    return *this;
}

auto bit_stream_writer::write(float data) -> bit_stream_writer&
{
    // Use the reinterpreted value of `data` as an u32
    std::uint32_t converted = std::bit_cast<std::uint32_t>(data);

    return write(converted);
}

auto bit_stream_writer::write(double data) -> bit_stream_writer&
{
    // Use the reinterpreted value of `data` as an u64
    std::uint64_t converted = std::bit_cast<std::uint64_t>(data);

    return write(converted);
}

void bit_stream_writer::flush_if_scratch_overflow()
{
    if (_scratch_index >= static_cast<int>(8 * sizeof(word_type)))
        do_flush_word_unchecked();
}

void bit_stream_writer::do_flush_word_unchecked()
{
    // Get the lower word bits to flush.
    word_type word = static_cast<word_type>((_scratch << (8 * sizeof(word_type))) >> (8 * sizeof(word_type)));
    if constexpr (std::endian::native == std::endian::big)
        word = std::byteswap(word);

    // Flush the word.
    _words[_words_index++] = word;

    // Remove the flushed scratch data.
    _scratch >>= (8 * sizeof(word_type));

    // Adjust the scratch index.
    _scratch_index = std::max(0, _scratch_index - static_cast<int>(8 * sizeof(word_type)));
}

bit_stream_reader::bit_stream_reader()
{
    reset();
}

bit_stream_reader::bit_stream_reader(std::span<const word_type> buffer, size_type logical_bytes_length)
{
    reset_with(buffer, logical_bytes_length);
}

bit_stream_reader::bit_stream_reader(const word_type* begin, const word_type* end, size_type logical_bytes_length)
{
    reset_with(begin, end, logical_bytes_length);
}

bit_stream_reader::bit_stream_reader(const word_type* begin, size_type words_length, size_type logical_bytes_length)
{
    reset_with(begin, words_length, logical_bytes_length);
}

void bit_stream_reader::restart()
{
    _scratch = 0;

    _scratch_bits = 0;
    _words_index = 0;

    _logical_used_bits = 0;
    _fail = _init_fail;
}

void bit_stream_reader::reset()
{
    _words = decltype(_words)();
    _logical_total_bits = 0;
    _init_fail = true;

    restart();
}

void bit_stream_reader::reset_with(std::span<const word_type> buffer, size_type logical_bytes_length)
{
    _words = buffer;
    _logical_total_bits = 8 * logical_bytes_length;
    _init_fail = (!buffer.data() || buffer.size() == 0 || std::size_t(logical_bytes_length) > buffer.size_bytes());

    restart();
}

void bit_stream_reader::reset_with(const word_type* begin, const word_type* end, size_type logical_bytes_length)
{
    reset_with(std::span<const word_type>(begin, end), logical_bytes_length);
}

void bit_stream_reader::reset_with(const word_type* begin, size_type words_length, size_type logical_bytes_length)
{
    reset_with(std::span<const word_type>(begin, words_length), logical_bytes_length);
}

auto bit_stream_reader::read(float& data) -> bit_stream_reader&
{
    // Read value as `u32`
    std::uint32_t raw;
    if (!read(raw))
        return *this;

    // Read to `data` by reinterpreting `raw` to float
    data = std::bit_cast<float>(raw);

    return *this;
}

auto bit_stream_reader::read(double& data) -> bit_stream_reader&
{
    // Read value as `u64`
    std::uint64_t raw;
    if (!read(raw))
        return *this;

    // Read to `data` by reinterpreting `raw` to double
    data = std::bit_cast<double>(raw);

    return *this;
}

auto bit_stream_reader::peek_string_length() -> ssize_type
{
    // Back up previous stream states
    const auto prev_scratch = _scratch;
    const auto prev_scratch_bits = _scratch_bits;
    const auto prev_words_index = _words_index;
    const auto prev_logical_used_bits = _logical_used_bits;

    // Read string length
    const auto result = read_string_length();

    // Restore previous stream states
    _scratch = prev_scratch;
    _scratch_bits = prev_scratch_bits;
    _words_index = prev_words_index;
    _logical_used_bits = prev_logical_used_bits;

    return result;
}

auto bit_stream_reader::read_string_length() -> ssize_type
{
    ssize_type result = -1;

    // Read prefix of string length prefix
    size_type len_of_len;
    if (do_read<true>(len_of_len, bit_stream_writer::MIN_STR_LEN_PREFIX_PREFIX,
                      bit_stream_writer::MAX_STR_LEN_PREFIX_PREFIX))
    {
        // Read string length prefix
        switch (len_of_len)
        {
        case size_type(0): {
            std::uint8_t len;
            if (do_read<true>(len))
                result = static_cast<ssize_type>(len);
            break;
        }
        case size_type(1): {
            std::uint16_t len;
            if (do_read<true>(len))
                result = static_cast<ssize_type>(len);
            break;
        }
        case size_type(2): {
            std::uint32_t len;
            if (do_read<true>(len))
                result = static_cast<ssize_type>(len);
            break;
        }
        case size_type(3): {
            std::uint64_t len;
            if (do_read<true>(len))
                result = static_cast<ssize_type>(len);
            break;
        }
        default:
            std::unreachable();
        }
    }

    return result;
}

void bit_stream_reader::do_fetch_word_unchecked()
{
    // Get the word to load to scratch.
    word_type word = _words[_words_index++];
    if constexpr (std::endian::native == std::endian::big)
        word = std::byteswap(word);

    // Load to scratch.
    _scratch |= (static_cast<scratch_type>(word) << _scratch_bits);

    // Adjust the scratch bits.
    _scratch_bits += 8 * sizeof(word_type);
}

} // namespace nalchi

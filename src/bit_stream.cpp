#include "nalchi/bit_stream.hpp"

#include "nalchi/payload.h"

#include "byteswap.hpp"
#include "math.hpp"

#include <algorithm>
#include <bit>

namespace nalchi
{

bit_stream_writer::bit_stream_writer(nalchi_payload buffer)
    : bit_stream_writer(std::span<word_type>(reinterpret_cast<word_type*>(buffer.ptr),
                                             ceil_to_multiple_of<sizeof(word_type)>(buffer.size) / sizeof(word_type)),
                        buffer.size)
{
}

bit_stream_writer::bit_stream_writer(std::span<word_type> buffer, int logical_bytes_length)
    : _words(buffer), _logical_total_bits(8 * logical_bytes_length),
      _init_fail(!buffer.data() || buffer.size() == 0 || std::size_t(logical_bytes_length) > buffer.size_bytes()), _fail(_init_fail)
{
    reset();
}

bit_stream_writer::bit_stream_writer(word_type* begin, word_type* end, int logical_bytes_length)
    : bit_stream_writer(std::span<word_type>(begin, end), logical_bytes_length)
{
}

bit_stream_writer::bit_stream_writer(word_type* begin, int words_length, int logical_bytes_length)
    : bit_stream_writer(std::span<word_type>(begin, words_length), logical_bytes_length)
{
}

bit_stream_writer::~bit_stream_writer()
{
    flush_final();
}

auto bit_stream_writer::used_bytes() const -> std::int64_t
{
    return ceil_to_multiple_of<8>(used_bits()) / 8;
}

void bit_stream_writer::reset()
{
    _scratch = 0;

    _scratch_index = 0;
    _words_index = 0;

    _logical_used_bits = 0;
    _fail = _init_fail;

    _final_flush = false;
}

auto bit_stream_writer::flush_final() -> bit_stream_writer&
{
    NALCHI_BIT_STREAM_RETURN_IF_STREAM_ALREADY_FAILED(*this);

    _final_flush = true;

    // No-op if nothing to flush
    if (_scratch_index > 0)
        flush_word_unchecked();

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
        flush_word_unchecked();
}

void bit_stream_writer::flush_word_unchecked()
{
    // Get the lower word bits to flush.
    word_type word = static_cast<word_type>((_scratch << (8 * sizeof(word_type))) >> (8 * sizeof(word_type)));
    if constexpr (std::endian::native == std::endian::big)
        word = byteswap(word);

    // Flush the word.
    _words[_words_index++] = word;

    // Remove the flushed scratch data.
    _scratch >>= (8 * sizeof(word_type));

    // Adjust the scratch index.
    _scratch_index = std::max(0, _scratch_index - static_cast<int>(8 * sizeof(word_type)));
}

} // namespace nalchi

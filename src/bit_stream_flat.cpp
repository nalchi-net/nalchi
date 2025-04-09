#include "nalchi/bit_stream_flat.hpp"

NALCHI_FLAT_API nalchi::bit_stream_writer* nalchi_bit_stream_writer_construct_default()
{
    return new nalchi::bit_stream_writer;
}

NALCHI_FLAT_API nalchi::bit_stream_writer* nalchi_bit_stream_writer_construct_with_shared_payload(
    nalchi::shared_payload buffer, nalchi::bit_stream_writer::size_type logical_bytes_length)
{
    return new nalchi::bit_stream_writer(buffer, logical_bytes_length);
}

NALCHI_FLAT_API nalchi::bit_stream_writer* nalchi_bit_stream_writer_construct_with_word_range(
    nalchi::bit_stream_writer::word_type* begin, nalchi::bit_stream_writer::word_type* end,
    nalchi::bit_stream_writer::size_type logical_bytes_length)
{
    return new nalchi::bit_stream_writer(begin, end, logical_bytes_length);
}

NALCHI_FLAT_API nalchi::bit_stream_writer* nalchi_bit_stream_writer_construct_with_word_ptr_and_length(
    nalchi::bit_stream_writer::word_type* begin, nalchi::bit_stream_writer::size_type words_length,
    nalchi::bit_stream_writer::size_type logical_bytes_length)
{
    return new nalchi::bit_stream_writer(begin, words_length, logical_bytes_length);
}

NALCHI_FLAT_API void nalchi_bit_stream_writer_destroy(nalchi::bit_stream_writer* self)
{
    delete self;
}

NALCHI_FLAT_API void nalchi_bit_stream_writer_set_fail(nalchi::bit_stream_writer* self)
{
    self->set_fail();
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_fail(const nalchi::bit_stream_writer* self)
{
    return self->fail();
}

NALCHI_FLAT_API auto nalchi_bit_stream_writer_total_bytes(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type
{
    return self->total_bytes();
}

NALCHI_FLAT_API auto nalchi_bit_stream_writer_total_bits(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type
{
    return self->total_bits();
}

NALCHI_FLAT_API auto nalchi_bit_stream_writer_used_bytes(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type
{
    return self->used_bytes();
}

NALCHI_FLAT_API auto nalchi_bit_stream_writer_used_bits(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type
{
    return self->used_bits();
}

NALCHI_FLAT_API auto nalchi_bit_stream_writer_unused_bytes(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type
{
    return self->unused_bytes();
}

NALCHI_FLAT_API auto nalchi_bit_stream_writer_unused_bits(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type
{
    return self->unused_bits();
}

NALCHI_FLAT_API void nalchi_bit_stream_writer_restart(nalchi::bit_stream_writer* self)
{
    self->restart();
}

NALCHI_FLAT_API void nalchi_bit_stream_writer_reset(nalchi::bit_stream_writer* self)
{
    self->reset();
}

NALCHI_FLAT_API void nalchi_bit_stream_writer_reset_with_shared_payload(
    nalchi::bit_stream_writer* self, nalchi::shared_payload buffer,
    nalchi::bit_stream_writer::size_type logical_bytes_length)
{
    self->reset_with(buffer, logical_bytes_length);
}

NALCHI_FLAT_API void nalchi_bit_stream_writer_reset_with_word_range(
    nalchi::bit_stream_writer* self, nalchi::bit_stream_writer::word_type* begin,
    nalchi::bit_stream_writer::word_type* end, nalchi::bit_stream_writer::size_type logical_bytes_length)
{
    self->reset_with(begin, end, logical_bytes_length);
}

NALCHI_FLAT_API void nalchi_bit_stream_writer_reset_with_word_ptr_and_length(
    nalchi::bit_stream_writer* self, nalchi::bit_stream_writer::word_type* begin,
    nalchi::bit_stream_writer::size_type words_length, nalchi::bit_stream_writer::size_type logical_bytes_length)
{
    self->reset_with(begin, words_length, logical_bytes_length);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_flush_final(nalchi::bit_stream_writer* self)
{
    return self->flush_final();
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_flushed(const nalchi::bit_stream_writer* self)
{
    return self->flushed();
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_bytes(nalchi::bit_stream_writer* self, const void* data,
                                                          nalchi::bit_stream_writer::size_type size)
{
    return self->write(data, size);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_bool(nalchi::bit_stream_writer* self, bool data)
{
    return self->write(data);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_s8(nalchi::bit_stream_writer* self, std::int8_t data,
                                                       std::int8_t min, std::int8_t max)
{
    return self->write(data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_u8(nalchi::bit_stream_writer* self, std::uint8_t data,
                                                       std::uint8_t min, std::uint8_t max)
{
    return self->write(data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_s16(nalchi::bit_stream_writer* self, std::int16_t data,
                                                        std::int16_t min, std::int16_t max)
{
    return self->write(data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_u16(nalchi::bit_stream_writer* self, std::uint16_t data,
                                                        std::uint16_t min, std::uint16_t max)
{
    return self->write(data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_s32(nalchi::bit_stream_writer* self, std::int32_t data,
                                                        std::int32_t min, std::int32_t max)
{
    return self->write(data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_u32(nalchi::bit_stream_writer* self, std::uint32_t data,
                                                        std::uint32_t min, std::uint32_t max)
{
    return self->write(data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_s64(nalchi::bit_stream_writer* self, std::int64_t data,
                                                        std::int64_t min, std::int64_t max)
{
    return self->write(data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_u64(nalchi::bit_stream_writer* self, std::uint64_t data,
                                                        std::uint64_t min, std::uint64_t max)
{
    return self->write(data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_float(nalchi::bit_stream_writer* self, float data)
{
    return self->write(data);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_double(nalchi::bit_stream_writer* self, double data)
{
    return self->write(data);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_ordinary_string(nalchi::bit_stream_writer* self, const char* str)
{
    return self->write(str);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_wide_string(nalchi::bit_stream_writer* self, const wchar_t* str)
{
    return self->write(str);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_utf8_string(nalchi::bit_stream_writer* self, const char8_t* str)
{
    return self->write(str);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_utf16_string(nalchi::bit_stream_writer* self, const char16_t* str)
{
    return self->write(str);
}

NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_utf32_string(nalchi::bit_stream_writer* self, const char32_t* str)
{
    return self->write(str);
}

NALCHI_FLAT_API nalchi::bit_stream_measurer* nalchi_bit_stream_measurer_construct()
{
    return new nalchi::bit_stream_measurer;
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_destroy(nalchi::bit_stream_measurer* self)
{
    delete self;
}

NALCHI_FLAT_API auto nalchi_bit_stream_measurer_used_bytes(const nalchi::bit_stream_measurer* self)
    -> nalchi::bit_stream_measurer::size_type
{
    return self->used_bytes();
}

NALCHI_FLAT_API auto nalchi_bit_stream_measurer_used_bits(const nalchi::bit_stream_measurer* self)
    -> nalchi::bit_stream_measurer::size_type
{
    return self->used_bits();
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_restart(nalchi::bit_stream_measurer* self)
{
    self->restart();
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_bytes(nalchi::bit_stream_measurer* self, const void* data,
                                                            nalchi::bit_stream_measurer::size_type size)
{
    self->write(data, size);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_bool(nalchi::bit_stream_measurer* self, bool data)
{
    self->write(data);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_s8(nalchi::bit_stream_measurer* self, std::int8_t data,
                                                         std::int8_t min, std::int8_t max)
{
    self->write(data, min, max);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_u8(nalchi::bit_stream_measurer* self, std::uint8_t data,
                                                         std::uint8_t min, std::uint8_t max)
{
    self->write(data, min, max);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_s16(nalchi::bit_stream_measurer* self, std::int16_t data,
                                                          std::int16_t min, std::int16_t max)
{
    self->write(data, min, max);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_u16(nalchi::bit_stream_measurer* self, std::uint16_t data,
                                                          std::uint16_t min, std::uint16_t max)
{
    self->write(data, min, max);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_s32(nalchi::bit_stream_measurer* self, std::int32_t data,
                                                          std::int32_t min, std::int32_t max)
{
    self->write(data, min, max);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_u32(nalchi::bit_stream_measurer* self, std::uint32_t data,
                                                          std::uint32_t min, std::uint32_t max)
{
    self->write(data, min, max);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_s64(nalchi::bit_stream_measurer* self, std::int64_t data,
                                                          std::int64_t min, std::int64_t max)
{
    self->write(data, min, max);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_u64(nalchi::bit_stream_measurer* self, std::uint64_t data,
                                                          std::uint64_t min, std::uint64_t max)
{
    self->write(data, min, max);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_float(nalchi::bit_stream_measurer* self, float data)
{
    self->write(data);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_double(nalchi::bit_stream_measurer* self, double data)
{
    self->write(data);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_ordinary_string(nalchi::bit_stream_measurer* self,
                                                                      const char* str)
{
    self->write(str);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_wide_string(nalchi::bit_stream_measurer* self, const wchar_t* str)
{
    self->write(str);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_utf8_string(nalchi::bit_stream_measurer* self, const char8_t* str)
{
    self->write(str);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_utf16_string(nalchi::bit_stream_measurer* self,
                                                                   const char16_t* str)
{
    self->write(str);
}

NALCHI_FLAT_API void nalchi_bit_stream_measurer_write_utf32_string(nalchi::bit_stream_measurer* self,
                                                                   const char32_t* str)
{
    self->write(str);
}

NALCHI_FLAT_API nalchi::bit_stream_reader* nalchi_bit_stream_reader_construct_default()
{
    return new nalchi::bit_stream_reader;
}

NALCHI_FLAT_API nalchi::bit_stream_reader* nalchi_bit_stream_reader_construct_with_word_range(
    const nalchi::bit_stream_reader::word_type* begin, const nalchi::bit_stream_reader::word_type* end,
    nalchi::bit_stream_reader::size_type logical_bytes_length)
{
    return new nalchi::bit_stream_reader(begin, end, logical_bytes_length);
}

NALCHI_FLAT_API nalchi::bit_stream_reader* nalchi_bit_stream_reader_construct_with_word_ptr_and_length(
    const nalchi::bit_stream_reader::word_type* begin, nalchi::bit_stream_reader::size_type words_length,
    nalchi::bit_stream_reader::size_type logical_bytes_length)
{
    return new nalchi::bit_stream_reader(begin, words_length, logical_bytes_length);
}

NALCHI_FLAT_API void nalchi_bit_stream_reader_destroy(nalchi::bit_stream_reader* self)
{
    delete self;
}

NALCHI_FLAT_API void nalchi_bit_stream_reader_set_fail(nalchi::bit_stream_reader* self)
{
    self->set_fail();
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_fail(const nalchi::bit_stream_reader* self)
{
    return self->fail();
}

NALCHI_FLAT_API auto nalchi_bit_stream_reader_total_bytes(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type
{
    return self->total_bytes();
}

NALCHI_FLAT_API auto nalchi_bit_stream_reader_total_bits(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type
{
    return self->total_bits();
}

NALCHI_FLAT_API auto nalchi_bit_stream_reader_used_bytes(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type
{
    return self->used_bytes();
}

NALCHI_FLAT_API auto nalchi_bit_stream_reader_used_bits(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type
{
    return self->used_bits();
}

NALCHI_FLAT_API auto nalchi_bit_stream_reader_unused_bytes(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type
{
    return self->unused_bytes();
}

NALCHI_FLAT_API auto nalchi_bit_stream_reader_unused_bits(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type
{
    return self->unused_bits();
}

NALCHI_FLAT_API void nalchi_bit_stream_reader_restart(nalchi::bit_stream_reader* self)
{
    return self->restart();
}

NALCHI_FLAT_API void nalchi_bit_stream_reader_reset(nalchi::bit_stream_reader* self)
{
    return self->reset();
}

NALCHI_FLAT_API void nalchi_bit_stream_reader_reset_with_word_range(
    nalchi::bit_stream_reader* self, const nalchi::bit_stream_reader::word_type* begin,
    const nalchi::bit_stream_reader::word_type* end, nalchi::bit_stream_reader::size_type logical_bytes_length)
{
    return self->reset_with(begin, end, logical_bytes_length);
}

NALCHI_FLAT_API void nalchi_bit_stream_reader_reset_with_word_ptr_and_length(
    nalchi::bit_stream_reader* self, const nalchi::bit_stream_reader::word_type* begin,
    nalchi::bit_stream_reader::size_type words_length, nalchi::bit_stream_reader::size_type logical_bytes_length)
{
    return self->reset_with(begin, words_length, logical_bytes_length);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_bytes(nalchi::bit_stream_reader* self, void* data,
                                                         nalchi::bit_stream_reader::size_type size)
{
    return self->read(data, size);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_bool(nalchi::bit_stream_reader* self, bool* data)
{
    return self->read(*data);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_s8(nalchi::bit_stream_reader* self, std::int8_t* data,
                                                      std::int8_t min, std::int8_t max)
{
    return self->read(*data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_u8(nalchi::bit_stream_reader* self, std::uint8_t* data,
                                                      std::uint8_t min, std::uint8_t max)
{
    return self->read(*data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_s16(nalchi::bit_stream_reader* self, std::int16_t* data,
                                                       std::int16_t min, std::int16_t max)
{
    return self->read(*data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_u16(nalchi::bit_stream_reader* self, std::uint16_t* data,
                                                       std::uint16_t min, std::uint16_t max)
{
    return self->read(*data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_s32(nalchi::bit_stream_reader* self, std::int32_t* data,
                                                       std::int32_t min, std::int32_t max)
{
    return self->read(*data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_u32(nalchi::bit_stream_reader* self, std::uint32_t* data,
                                                       std::uint32_t min, std::uint32_t max)
{
    return self->read(*data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_s64(nalchi::bit_stream_reader* self, std::int64_t* data,
                                                       std::int64_t min, std::int64_t max)
{
    return self->read(*data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_u64(nalchi::bit_stream_reader* self, std::uint64_t* data,
                                                       std::uint64_t min, std::uint64_t max)
{
    return self->read(*data, min, max);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_float(nalchi::bit_stream_reader* self, float* data)
{
    return self->read(*data);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_double(nalchi::bit_stream_reader* self, double* data)
{
    return self->read(*data);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_ordinary_string(nalchi::bit_stream_reader* self, char* str,
                                                                   nalchi::bit_stream_reader::size_type max_length)
{
    return self->read(str, max_length);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_wide_string(nalchi::bit_stream_reader* self, wchar_t* str,
                                                               nalchi::bit_stream_reader::size_type max_length)
{
    return self->read(str, max_length);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_utf8_string(nalchi::bit_stream_reader* self, char8_t* str,
                                                               nalchi::bit_stream_reader::size_type max_length)
{
    return self->read(str, max_length);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_utf16_string(nalchi::bit_stream_reader* self, char16_t* str,
                                                                nalchi::bit_stream_reader::size_type max_length)
{
    return self->read(str, max_length);
}

NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_utf32_string(nalchi::bit_stream_reader* self, char32_t* str,
                                                                nalchi::bit_stream_reader::size_type max_length)
{
    return self->read(str, max_length);
}

NALCHI_FLAT_API auto nalchi_bit_stream_reader_peek_string_length(nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::ssize_type
{
    return self->peek_string_length();
}

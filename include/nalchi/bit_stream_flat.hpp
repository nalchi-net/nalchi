/// @file
/// @brief Bit stream flat API.

#pragma once

#include "nalchi/bit_stream.hpp"

#include "nalchi/export.hpp"

#include <cstdint>

/// @brief Constructs a `bit_stream_writer` instance without a buffer.
///
/// This constructor can be useful if you want to set the buffer afterwards. \n
/// To set the buffer, call `reset_with()`.
NALCHI_FLAT_API nalchi::bit_stream_writer* nalchi_bit_stream_writer_construct_default();

/// @brief Constructs a `bit_stream_writer` instance with a `shared_payload` buffer.
/// @param buffer Buffer to write bits to.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial write to the final word.
NALCHI_FLAT_API nalchi::bit_stream_writer* nalchi_bit_stream_writer_construct_with_shared_payload(
    nalchi::shared_payload buffer, nalchi::bit_stream_writer::size_type logical_bytes_length);

/// @brief Constructs a `bit_stream_writer` instance with a word range.
/// @param begin Pointer to the beginning of a buffer.
/// @param end Pointer to the end of a buffer.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial write to the final word.
NALCHI_FLAT_API nalchi::bit_stream_writer* nalchi_bit_stream_writer_construct_with_word_range(
    nalchi::bit_stream_writer::word_type* begin, nalchi::bit_stream_writer::word_type* end,
    nalchi::bit_stream_writer::size_type logical_bytes_length);

/// @brief Constructs a `bit_stream_writer` instance with a word begin pointer and the word length.
/// @param begin Pointer to the beginning of a buffer.
/// @param words_length Number of words in the buffer.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial write to the final word.
NALCHI_FLAT_API nalchi::bit_stream_writer* nalchi_bit_stream_writer_construct_with_word_ptr_and_length(
    nalchi::bit_stream_writer::word_type* begin, nalchi::bit_stream_writer::size_type words_length,
    nalchi::bit_stream_writer::size_type logical_bytes_length);

/// @brief Destroys the `bit_stream_writer` instance.
NALCHI_FLAT_API void nalchi_bit_stream_writer_destroy(nalchi::bit_stream_writer* self);

/// @brief Check if writing to your buffer has been failed or not.
///
/// If this is `true`, all the operations for this `bit_stream_writer` is no-op.
/// @return `true` if writing has been failed, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_fail(const nalchi::bit_stream_writer* self);

/// @brief Gets the number of total bytes in the stream.
/// @return Number of total bytes in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_writer_total_bytes(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type;

/// @brief Gets the number of total bits in the stream.
/// @return Number of total bits in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_writer_total_bits(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type;

/// @brief Gets the number of used bytes in the stream.
/// @return Number of used bytes in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_writer_used_bytes(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type;

/// @brief Gets the number of used bits in the stream.
/// @return Number of used bits in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_writer_used_bits(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type;

/// @brief Gets the number of unused bytes in the stream.
/// @return Number of unused bytes in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_writer_unused_bytes(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type;

/// @brief Gets the number of unused bits in the stream.
/// @return Number of unused bits in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_writer_unused_bits(const nalchi::bit_stream_writer* self)
    -> nalchi::bit_stream_writer::size_type;

/// @brief Restarts the stream so that it can write from the beginning again.
/// @note This function resets internal states @b without flushing,
/// so if you need flushing, you should call `flush_final()` beforehand.
NALCHI_FLAT_API void nalchi_bit_stream_writer_restart(nalchi::bit_stream_writer* self);

/// @brief Resets the stream so that it no longer holds your buffer anymore.
/// @note This function removes reference to your buffer @b without flushing, \n
/// so if you need flushing, you should call `flush_final()` beforehand.
NALCHI_FLAT_API void nalchi_bit_stream_writer_reset(nalchi::bit_stream_writer* self);

/// @brief Resets the stream with a `shared_payload` buffer.
/// @note This function resets to the new buffer @b without flushing to your previous buffer, \n
/// so if you need flushing, you should call `flush_final()` beforehand.
/// @param buffer Buffer to write bits to.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial write to the final word.
NALCHI_FLAT_API void nalchi_bit_stream_writer_reset_with_shared_payload(
    nalchi::bit_stream_writer* self, nalchi::shared_payload buffer,
    nalchi::bit_stream_writer::size_type logical_bytes_length);

/// @brief Resets the stream with a word range.
/// @note This function resets to the new buffer @b without flushing to your previous buffer, \n
/// so if you need flushing, you should call `flush_final()` beforehand.
/// @param begin Pointer to the beginning of a buffer.
/// @param end Pointer to the end of a buffer.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial write to the final word.
NALCHI_FLAT_API void nalchi_bit_stream_writer_reset_with_word_range(
    nalchi::bit_stream_writer* self, nalchi::bit_stream_writer::word_type* begin,
    nalchi::bit_stream_writer::word_type* end, nalchi::bit_stream_writer::size_type logical_bytes_length);

/// @brief Resets the stream with a word begin pointer and the word length.
/// @note This function resets to the new buffer @b without flushing to your previous buffer, \n
/// so if you need flushing, you should call `flush_final()` beforehand.
/// @param begin Pointer to the beginning of a buffer.
/// @param words_length Number of words in the buffer.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial write to the final word.
NALCHI_FLAT_API void nalchi_bit_stream_writer_reset_with_word_ptr_and_length(
    nalchi::bit_stream_writer* self, nalchi::bit_stream_writer::word_type* begin,
    nalchi::bit_stream_writer::size_type words_length, nalchi::bit_stream_writer::size_type logical_bytes_length);

/// @brief Flushes the last remaining bytes on the internal scratch buffer to your buffer.
/// @note This function must be only called when you're done writing. \n
/// Any attempt to write more data after calling this function will set the fail flag and write nothing.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_flush_final(nalchi::bit_stream_writer* self);

/// @brief Checks if `flush_final()` has been called or not.
/// @return Whether the `flush_final()` has been called or not.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_flushed(const nalchi::bit_stream_writer* self);

/// @brief Writes some arbitrary data to the bit stream.
/// @note Bytes in your data could be read @b swapped if it is sent to the system with different endianness. \n
/// So, prefer using other overloads instead.
/// @param data Pointer to the arbitrary data.
/// @param size Size in bytes of the data.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_bytes(nalchi::bit_stream_writer* self, const void* data,
                                                          nalchi::bit_stream_writer::size_type size);

/// @brief Writes a `std::int8_t` value to the bit stream.
/// @param data Data to write.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_s8(nalchi::bit_stream_writer* self, std::int8_t data,
                                                       std::int8_t min, std::int8_t max);

/// @brief Writes a `std::uint8_t` value to the bit stream.
/// @param data Data to write.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_u8(nalchi::bit_stream_writer* self, std::uint8_t data,
                                                       std::uint8_t min, std::uint8_t max);

/// @brief Writes a `std::int16_t` value to the bit stream.
/// @param data Data to write.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_s16(nalchi::bit_stream_writer* self, std::int16_t data,
                                                        std::int16_t min, std::int16_t max);

/// @brief Writes a `std::uint16_t` value to the bit stream.
/// @param data Data to write.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_u16(nalchi::bit_stream_writer* self, std::uint16_t data,
                                                        std::uint16_t min, std::uint16_t max);

/// @brief Writes a `std::int32_t` value to the bit stream.
/// @param data Data to write.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_s32(nalchi::bit_stream_writer* self, std::int32_t data,
                                                        std::int32_t min, std::int32_t max);

/// @brief Writes a `std::uint32_t` value to the bit stream.
/// @param data Data to write.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_u32(nalchi::bit_stream_writer* self, std::uint32_t data,
                                                        std::uint32_t min, std::uint32_t max);

/// @brief Writes a `std::int64_t` value to the bit stream.
/// @param data Data to write.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_s64(nalchi::bit_stream_writer* self, std::int64_t data,
                                                        std::int64_t min, std::int64_t max);

/// @brief Writes a `std::uint64_t` value to the bit stream.
/// @param data Data to write.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_u64(nalchi::bit_stream_writer* self, std::uint64_t data,
                                                        std::uint64_t min, std::uint64_t max);

/// @brief Writes a float value to the bit stream.
/// @param data Data to write.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_float(nalchi::bit_stream_writer* self, float data);

/// @brief Writes a double value to the bit stream.
/// @param data Data to write.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_double(nalchi::bit_stream_writer* self, double data);

/// @brief Writes a null-terminated ordinary string to the bit stream.
/// @param str String to write.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_ordinary_string(nalchi::bit_stream_writer* self, const char* str);

/// @brief Writes a null-terminated wide string to the bit stream.
/// @param str String to write.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_wide_string(nalchi::bit_stream_writer* self, const wchar_t* str);

/// @brief Writes a null-terminated UTF-8 string to the bit stream.
/// @param str String to write.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_utf8_string(nalchi::bit_stream_writer* self, const char8_t* str);

/// @brief Writes a null-terminated UTF-16 string to the bit stream.
/// @param str String to write.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_utf16_string(nalchi::bit_stream_writer* self, const char16_t* str);

/// @brief Writes a null-terminated UTF-32 string to the bit stream.
/// @param str String to write.
/// @return `true` if writing has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_writer_write_utf32_string(nalchi::bit_stream_writer* self, const char32_t* str);

/// @brief Constructs a `bit_stream_reader` instance without a buffer.
///
/// This constructor can be useful if you want to set the buffer afterwards. \n
/// To set the buffer, call `reset_with()`.
NALCHI_FLAT_API nalchi::bit_stream_reader* nalchi_bit_stream_reader_construct_default();

/// @brief Constructs a `bit_stream_reader` instance with a word range.
/// @param begin Pointer to the beginning of a buffer.
/// @param end Pointer to the end of a buffer.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial read from the final word.
NALCHI_FLAT_API nalchi::bit_stream_reader* nalchi_bit_stream_reader_construct_with_word_range(
    const nalchi::bit_stream_reader::word_type* begin, const nalchi::bit_stream_reader::word_type* end,
    nalchi::bit_stream_reader::size_type logical_bytes_length);

/// @brief Constructs a `bit_stream_reader` instance with a word begin pointer and the word length.
/// @param begin Pointer to the beginning of a buffer.
/// @param words_length Number of words in the buffer.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial read from the final word.
NALCHI_FLAT_API nalchi::bit_stream_reader* nalchi_bit_stream_reader_construct_with_word_ptr_and_length(
    const nalchi::bit_stream_reader::word_type* begin, nalchi::bit_stream_reader::size_type words_length,
    nalchi::bit_stream_reader::size_type logical_bytes_length);

/// @brief Destroys the `bit_stream_reader` instance.
NALCHI_FLAT_API void nalchi_bit_stream_reader_destroy(nalchi::bit_stream_reader* self);

/// @brief Check if reading from your buffer has been failed or not.
///
/// If this is `true`, all the operations for this `bit_stream_reader` is no-op.
/// @return `true` if reading has been failed, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_fail(const nalchi::bit_stream_reader* self);

/// @brief Gets the number of total bytes in the stream.
/// @return Number of total bytes in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_reader_total_bytes(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type;

/// @brief Gets the number of total bits in the stream.
/// @return Number of total bits in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_reader_total_bits(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type;

/// @brief Gets the number of used bytes in the stream.
/// @return Number of used bytes in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_reader_used_bytes(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type;

/// @brief Gets the number of used bits in the stream.
/// @return Number of used bits in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_reader_used_bits(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type;

/// @brief Gets the number of unused bytes in the stream.
/// @return Number of unused bytes in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_reader_unused_bytes(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type;

/// @brief Gets the number of unused bits in the stream.
/// @return Number of unused bits in the stream.
NALCHI_FLAT_API auto nalchi_bit_stream_reader_unused_bits(const nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::size_type;

/// @brief Restarts the stream so that it can read from the beginning again.
NALCHI_FLAT_API void nalchi_bit_stream_reader_restart(nalchi::bit_stream_reader* self);

/// @brief Resets the stream so that it no longer holds your buffer anymore.
NALCHI_FLAT_API void nalchi_bit_stream_reader_reset(nalchi::bit_stream_reader* self);

/// @brief Resets the stream with a word range.
/// @param begin Pointer to the beginning of a buffer.
/// @param end Pointer to the end of a buffer.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial read from the final word.
NALCHI_FLAT_API void nalchi_bit_stream_reader_reset_with_word_range(
    nalchi::bit_stream_reader* self, const nalchi::bit_stream_reader::word_type* begin,
    const nalchi::bit_stream_reader::word_type* end, nalchi::bit_stream_reader::size_type logical_bytes_length);

/// @brief Resets the stream with a word begin pointer and the word length.
/// @param begin Pointer to the beginning of a buffer.
/// @param words_length Number of words in the buffer.
/// @param logical_bytes_length Number of bytes logically.
/// This is useful if you want to only allow partial read from the final word.
NALCHI_FLAT_API void nalchi_bit_stream_reader_reset_with_word_ptr_and_length(
    nalchi::bit_stream_reader* self, const nalchi::bit_stream_reader::word_type* begin,
    nalchi::bit_stream_reader::size_type words_length, nalchi::bit_stream_reader::size_type logical_bytes_length);

/// @brief Reads some arbitrary data from the bit stream.
/// @note You could read @b swapped bytes if the data came from the system with different endianness. \n
/// So, prefer using other overloads instead.
/// @param data Pointer to the arbitrary data.
/// @param size Size in bytes of the data.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_bytes(nalchi::bit_stream_reader* self, void* data,
                                                         nalchi::bit_stream_reader::size_type size);

/// @brief Reads a `std::int8_t` value from the bit stream.
/// @param data Data to read to.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_s8(nalchi::bit_stream_reader* self, std::int8_t* data,
                                                      std::int8_t min, std::int8_t max);

/// @brief Reads a `std::uint8_t` value from the bit stream.
/// @param data Data to read to.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_u8(nalchi::bit_stream_reader* self, std::uint8_t* data,
                                                      std::uint8_t min, std::uint8_t max);

/// @brief Reads a `std::int16_t` value from the bit stream.
/// @param data Data to read to.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_s16(nalchi::bit_stream_reader* self, std::int16_t* data,
                                                       std::int16_t min, std::int16_t max);

/// @brief Reads a `std::uint16_t` value from the bit stream.
/// @param data Data to read to.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_u16(nalchi::bit_stream_reader* self, std::uint16_t* data,
                                                       std::uint16_t min, std::uint16_t max);

/// @brief Reads a `std::int32_t` value from the bit stream.
/// @param data Data to read to.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_s32(nalchi::bit_stream_reader* self, std::int32_t* data,
                                                       std::int32_t min, std::int32_t max);

/// @brief Reads a `std::uint32_t` value from the bit stream.
/// @param data Data to read to.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_u32(nalchi::bit_stream_reader* self, std::uint32_t* data,
                                                       std::uint32_t min, std::uint32_t max);

/// @brief Reads a `std::int64_t` value from the bit stream.
/// @param data Data to read to.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_s64(nalchi::bit_stream_reader* self, std::int64_t* data,
                                                       std::int64_t min, std::int64_t max);

/// @brief Reads a `std::uint64_t` value from the bit stream.
/// @param data Data to read to.
/// @param min Minimum value allowed for @p data.
/// @param max Maximum value allowed for @p data.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_u64(nalchi::bit_stream_reader* self, std::uint64_t* data,
                                                       std::uint64_t min, std::uint64_t max);

/// @brief Reads a float value from the bit stream.
/// @param data Data to read to.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_float(nalchi::bit_stream_reader* self, float* data);

/// @brief Reads a double value from the bit stream.
/// @param data Data to read to.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_double(nalchi::bit_stream_reader* self, double* data);

/// @brief Reads a null-terminated ordinary string from the bit stream.
///
/// If @p max_length is not enough to store the string, \n
/// this function will set the fail flag and read nothing.
/// @note @p max_length does @b not include null character, so your buffer @b must allocate additional space for it.
///
/// For example, if @p max_length is 4 for `char`, you need 5 bytes. \n
/// Because you need space for 5 `char` including null char, and `char` is 1 byte per char.
/// @param str Null-terminated string to read to.
/// @param max_length Maximum number of `char` that can be read.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_ordinary_string(nalchi::bit_stream_reader* self, char* str,
                                                                   nalchi::bit_stream_reader::size_type max_length);

/// @brief Reads a null-terminated wide string from the bit stream.
///
/// If @p max_length is not enough to store the string, \n
/// this function will set the fail flag and read nothing.
/// @note @p max_length does @b not include null character, so your buffer @b must allocate additional space for it.
///
/// For example, if @p max_length is 4 for `char16_t`, you need 10 bytes. \n
/// Because you need space for 5 `char16_t` including null char, and `char16_t` is 2 bytes per char.
/// @param str Null-terminated string to read to.
/// @param max_length Maximum number of `wchar_t` that can be read.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_wide_string(nalchi::bit_stream_reader* self, wchar_t* str,
                                                               nalchi::bit_stream_reader::size_type max_length);

/// @brief Reads a null-terminated UTF-8 string from the bit stream.
///
/// If @p max_length is not enough to store the string, \n
/// this function will set the fail flag and read nothing.
/// @note @p max_length does @b not include null character, so your buffer @b must allocate additional space for it.
///
/// For example, if @p max_length is 4 for `char8_t`, you need 5 bytes. \n
/// Because you need space for 5 `char8_t` including null char, and `char8_t` is 1 byte per char.
/// @param str Null-terminated string to read to.
/// @param max_length Maximum number of `char8_t` that can be read.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_utf8_string(nalchi::bit_stream_reader* self, char8_t* str,
                                                               nalchi::bit_stream_reader::size_type max_length);

/// @brief Reads a null-terminated UTF-16 string from the bit stream.
///
/// If @p max_length is not enough to store the string, \n
/// this function will set the fail flag and read nothing.
/// @note @p max_length does @b not include null character, so your buffer @b must allocate additional space for it.
///
/// For example, if @p max_length is 4 for `char16_t`, you need 10 bytes. \n
/// Because you need space for 5 `char16_t` including null char, and `char16_t` is 2 bytes per char.
/// @param str Null-terminated string to read to.
/// @param max_length Maximum number of `char16_t` that can be read.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_utf16_string(nalchi::bit_stream_reader* self, char16_t* str,
                                                                nalchi::bit_stream_reader::size_type max_length);

/// @brief Reads a null-terminated UTF-32 string from the bit stream.
///
/// If @p max_length is not enough to store the string, \n
/// this function will set the fail flag and read nothing.
/// @note @p max_length does @b not include null character, so your buffer @b must allocate additional space for it.
///
/// For example, if @p max_length is 4 for `char32_t`, you need 20 bytes. \n
/// Because you need space for 5 `char32_t` including null char, and `char32_t` is 4 bytes per char.
/// @param str Null-terminated string to read to.
/// @param max_length Maximum number of `char32_t` that can be read.
/// @return `true` if reading has been successful, otherwise `false`.
NALCHI_FLAT_API bool nalchi_bit_stream_reader_read_utf32_string(nalchi::bit_stream_reader* self, char32_t* str,
                                                                nalchi::bit_stream_reader::size_type max_length);

/// @brief Peeks the string length prefix from the current stream position.
///
/// If it fails to read a string length prefix, \n
/// this function will return a negative value and set the fail flag.
/// @note Be careful, if current stream position was not on the string length prefix, it might read garbage length!
/// @return Length of `CharT` stored in it, or a negative value if length prefix is invalid.
NALCHI_FLAT_API auto nalchi_bit_stream_reader_peek_string_length(nalchi::bit_stream_reader* self)
    -> nalchi::bit_stream_reader::ssize_type;

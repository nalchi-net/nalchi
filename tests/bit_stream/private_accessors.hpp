#pragma once

#include <nalchi/bit_stream.hpp>

namespace nalchi::tests
{

// Hack to access private variables of an arbitrary class.
// See https://www.worldcadaccess.com/blog/2020/05/how-to-hack-c-with-templates-and-friends.html
template <bit_stream_writer::scratch_type bit_stream_writer::* Scratch,
          std::span<bit_stream_writer::word_type> bit_stream_writer::* Words, int bit_stream_writer::* ScratchIndex,
          int bit_stream_writer::* WordsIndex, bit_stream_writer::size_type bit_stream_writer::* LogicalTotalBits,
          bit_stream_writer::size_type bit_stream_writer::* LogicalUsedBits, bool bit_stream_writer::* InitFail,
          bool bit_stream_writer::* Fail, bool bit_stream_writer::* FinalFlushed>
struct bit_stream_writer_private_accessor
{
    friend auto get_writer_scratch(bit_stream_writer& writer) -> bit_stream_writer::scratch_type&
    {
        return writer.*Scratch;
    }

    friend auto get_writer_words(bit_stream_writer& writer) -> std::span<bit_stream_writer::word_type>&
    {
        return writer.*Words;
    }

    friend int& get_writer_scratch_index(bit_stream_writer& writer)
    {
        return writer.*ScratchIndex;
    }

    friend int& get_writer_words_index(bit_stream_writer& writer)
    {
        return writer.*WordsIndex;
    }

    friend auto get_writer_logical_total_bits(bit_stream_writer& writer) -> bit_stream_writer::size_type&
    {
        return writer.*LogicalTotalBits;
    }

    friend auto get_writer_logical_used_bits(bit_stream_writer& writer) -> bit_stream_writer::size_type&
    {
        return writer.*LogicalUsedBits;
    }

    friend bool& get_writer_init_fail(bit_stream_writer& writer)
    {
        return writer.*InitFail;
    }

    friend bool& get_writer_fail(bit_stream_writer& writer)
    {
        return writer.*Fail;
    }

    friend bool& get_writer_final_flushed(bit_stream_writer& writer)
    {
        return writer.*FinalFlushed;
    }
};

template <bit_stream_reader::scratch_type bit_stream_reader::* Scratch,
          std::span<const bit_stream_reader::word_type> bit_stream_reader::* Words,
          int bit_stream_reader::* ScratchBits, int bit_stream_reader::* WordsIndex,
          bit_stream_reader::size_type bit_stream_reader::* LogicalTotalBits,
          bit_stream_reader::size_type bit_stream_reader::* LogicalUsedBits, bool bit_stream_reader::* InitFail,
          bool bit_stream_reader::* Fail>
struct bit_stream_reader_private_accessor
{
    friend auto get_reader_scratch(bit_stream_reader& reader) -> bit_stream_reader::scratch_type&
    {
        return reader.*Scratch;
    }

    friend auto get_reader_words(bit_stream_reader& reader) -> std::span<const bit_stream_reader::word_type>&
    {
        return reader.*Words;
    }

    friend int& get_reader_scratch_bits(bit_stream_reader& reader)
    {
        return reader.*ScratchBits;
    }

    friend int& get_reader_words_index(bit_stream_reader& reader)
    {
        return reader.*WordsIndex;
    }

    friend auto get_reader_logical_total_bits(bit_stream_reader& reader) -> bit_stream_reader::size_type&
    {
        return reader.*LogicalTotalBits;
    }

    friend auto get_reader_logical_used_bits(bit_stream_reader& reader) -> bit_stream_reader::size_type&
    {
        return reader.*LogicalUsedBits;
    }

    friend bool& get_reader_init_fail(bit_stream_reader& reader)
    {
        return reader.*InitFail;
    }

    friend bool& get_reader_fail(bit_stream_reader& reader)
    {
        return reader.*Fail;
    }
};

template struct bit_stream_writer_private_accessor<
    &bit_stream_writer::_scratch, &bit_stream_writer::_words, &bit_stream_writer::_scratch_index,
    &bit_stream_writer::_words_index, &bit_stream_writer::_logical_total_bits, &bit_stream_writer::_logical_used_bits,
    &bit_stream_writer::_init_fail, &bit_stream_writer::_fail, &bit_stream_writer::_final_flushed>;

template struct bit_stream_reader_private_accessor<
    &bit_stream_reader::_scratch, &bit_stream_reader::_words, &bit_stream_reader::_scratch_bits,
    &bit_stream_reader::_words_index, &bit_stream_reader::_logical_total_bits, &bit_stream_reader::_logical_used_bits,
    &bit_stream_reader::_init_fail, &bit_stream_reader::_fail>;

auto get_writer_scratch(bit_stream_writer&) -> bit_stream_writer::scratch_type&;
auto get_writer_words(bit_stream_writer& writer) -> std::span<bit_stream_writer::word_type>&;
int& get_writer_scratch_index(bit_stream_writer& writer);
int& get_writer_words_index(bit_stream_writer& writer);
auto get_writer_logical_total_bits(bit_stream_writer& writer) -> bit_stream_writer::size_type&;
auto get_writer_logical_used_bits(bit_stream_writer& writer) -> bit_stream_writer::size_type&;
bool& get_writer_init_fail(bit_stream_writer& writer);
bool& get_writer_fail(bit_stream_writer& writer);
bool& get_writer_final_flushed(bit_stream_writer& writer);

auto get_reader_scratch(bit_stream_reader& reader) -> bit_stream_reader::scratch_type&;
auto get_reader_words(bit_stream_reader& reader) -> std::span<const bit_stream_reader::word_type>&;
int& get_reader_scratch_bits(bit_stream_reader& reader);
int& get_reader_words_index(bit_stream_reader& reader);
auto get_reader_logical_total_bits(bit_stream_reader& reader) -> bit_stream_reader::size_type&;
auto get_reader_logical_used_bits(bit_stream_reader& reader) -> bit_stream_reader::size_type&;
bool& get_reader_init_fail(bit_stream_reader& reader);
bool& get_reader_fail(bit_stream_reader& reader);

} // namespace nalchi::tests

#pragma once

#include "byte_stream.hh"
#include<map>

class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler( ByteStream&& output ) 
  : output_( std::move( output ) ),
    unassembled_segments_(),
    next_index_( 0 ),
    bytes_pending_( 0 ),
    eof_flag_( false ),
    eof_index_( 0 )
  {}

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );
  void store_segment(uint64_t first_index,std::string &data);
  void try_assemble();
  void push_output(std::string &data);

  // How many bytes are stored in the Reassembler itself?
  // This function is for testing only; don't add extra state to support it.
  uint64_t count_bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_;

  std::map<uint64_t,std::string> unassembled_segments_;
  // 起到缓冲区的作用，保留那些还没被处理到output_流当中的段

  uint64_t next_index_=0;
  // 下一个期望写入ByteStream的索引是什么？

  uint64_t  bytes_pending_=0;
  // 时刻记录缓冲区里面的数量

  bool eof_flag_=false;
  // 记录有没有收到最后那个段

  uint64_t eof_index_=0;
  // 如果收到了最后的段，那么最后那个段的start_index就记作结束索引



};

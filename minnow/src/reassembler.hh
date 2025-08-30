#pragma once

#include "byte_stream.hh"
#include <map>

// 重排器，把收到的段重新排序
class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  // 构造函数，给出要写入的ByteStream(output)
  // 记录未排序的段数
  // 记录期望收到的下一个段编号
  // 记录缓冲区当中的字节数
  // 记录流是否结束
  // 记录流结束的索引
  explicit Reassembler( ByteStream&& output )
    : output_( std::move( output ) )
    , unassembled_segments_()
    , next_index_( 0 )
    , bytes_pending_( 0 )
    , eof_flag_( false )
    , eof_index_( 0 )
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

  // 插入(接受)一个段
  void insert( uint64_t first_index, std::string data, bool is_last_substring );
  // 把段存入缓冲区
  void store_segment( uint64_t first_index, std::string& data );
  // 尝试看看缓冲区中是否有段可以合并
  void try_assemble();
  // 推送进入output(流)
  void push_output( std::string& data );

  // 注意这里实际上有两个"缓冲区"的概念，重排器自己有一个缓冲区
  // 流有一个buffer缓冲区

  // How many bytes are stored in the Reassembler itself?
  // This function is for testing only; don't add extra state to support it.
  // 下面是一些帮助函数
  uint64_t count_bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

  void increment_next_index();

private:
  // 流，重排器把段写入这个流
  ByteStream output_;

  std::map<uint64_t, std::string> unassembled_segments_;
  // 起到缓冲区的作用，保留那些还没被处理到output_流当中的段

  uint64_t next_index_ = 0;
  // 下一个期望写入ByteStream的索引是什么？

  uint64_t bytes_pending_ = 0;
  // 时刻记录缓冲区里面的数量

  bool eof_flag_ = false;
  // 记录有没有收到最后那个段

  uint64_t eof_index_ = 0;
  // 如果收到了最后的段，那么最后那个段的start_index就记作结束索引
};

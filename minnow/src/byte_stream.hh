#pragma once

#include <cstdint>
#include <string>
#include <string_view>
// 什么是向TCP协议栈写入的“流”？
// 流是一个类，也可以说是一种数据结构
// 通过Reader和Writer去控制它
// 这是为了避免耦合，用子类去控制接口
class Reader;
class Writer;

class ByteStream
{
public:
  explicit ByteStream( uint64_t capacity );
  // 构造，一个流当然要用capacity来指定里面最多放多少数据

  // Helper functions (provided) to access the ByteStream's Reader and Writer interfaces
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;
  // 获取ByteStream对象的Reader和Writer接口
  // const放前面表示返回的是Reader/Writer的常量引用
  // 我不能通过这个引用去修改Reader对象本身
  // const放后面说明这是一个ByteStream类的常量成员函数
  // 它向编译器承诺不修改ByteStream的任何成员变量

  void set_error() { error_ = true; };       // Signal that the stream suffered an error.
  // 通过设置this->error_=true
  // 来说明这个流出错了
  bool has_error() const { return error_; }; // Has the stream had an error?
  // 类似上面的set_error()

protected:
  // Please add any additional state to the ByteStream here, and not to the Writer and Reader interfaces.
  uint64_t capacity_;
  // 容量
  bool error_ {};
  // 错误标志

  std::string buffer_;
  // 缓冲区，表现为字符串
  uint64_t bytes_pushed_ = 0;
  // 已注入的字节数
  uint64_t bytes_popped_ = 0;
  // 已出栈的字节数
  bool is_closed_ = false;
  // 流是否已关闭
};

class Writer : public ByteStream
// 写入ByteStream的接口，显式继承自ByteStream
{
public:
  void push( std::string data ); // Push data to stream, but only as much as available capacity allows.
  // 往缓冲区注入string
  void close();                  // Signal that the stream has reached its ending. Nothing more will be written.
  // 关闭流

  bool is_closed() const;              // Has the stream been closed?
  // 流是否关闭？
  uint64_t available_capacity() const; // How many bytes can be pushed to the stream right now?
  // 流的缓冲区还剩多少容量？
  uint64_t bytes_pushed() const;       // Total number of bytes cumulatively pushed to the stream
  // 流的累计注入字节数
};

class Reader : public ByteStream
// 读取ByteStream的接口，显式继承自ByteStream
{
public:
  std::string_view peek() const; // Peek at the next bytes in the buffer
  // 查看缓冲区里的string,但是是以非拥有性质的

  void pop( uint64_t len );      // Remove `len` bytes from the buffer
  // 出栈len个字节

  bool is_finished() const;        // Is the stream finished (closed and fully popped)?
  // 流是否已经结束？(和关闭有些区别)
  uint64_t bytes_buffered() const; // Number of bytes currently buffered (pushed and not popped)
  // 目前流中被缓存的字节数
  uint64_t bytes_popped() const;   // Total number of bytes cumulatively popped from stream
  // 流的累计出栈字节数
};

/*
 * read: A (provided) helper function thats peeks and pops up to `max_len` bytes
 * from a ByteStream Reader into a string;
 */
void read( Reader& reader, uint64_t max_len, std::string& out );
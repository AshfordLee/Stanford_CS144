#include "byte_stream.hh"

#include <cstdint>
#include <stdexcept>

using namespace std;

/*
 * read: A helper function thats peeks and pops up to `max_len` bytes
 * from a ByteStream Reader into a string;
 */
// 把缓冲区里面的东西读出来
void read( Reader& reader, uint64_t max_len, string& out )
{
  out.clear();

  while ( reader.bytes_buffered() and out.size() < max_len ) {
    // 每次都获取缓冲区的全体view
    auto view = reader.peek();

    if ( view.empty() ) {
      throw runtime_error( "Reader::peek() returned empty string_view" );
    }

    // 但是每次只截取至多max_len-out.size()个字节拿出来
    view = view.substr( 0, max_len - out.size() ); // Don't return more bytes than desired.

    // 合并
    out += view;

    // 把已经拿出来加入到out里面的那一部分弹出
    reader.pop( view.size() );
  }
}

// 获取ByteStream的reader接口，或者说重载了ByteStream的reader()函数
// 当我们输入ByteStream_object.reader()的时候，就直接拿到了这个接口

Reader& ByteStream::reader()
{
  // reader是Bytestream的接口，所以他只能有成员函数
  // 而不能有成员变量
  static_assert( sizeof( Reader ) == sizeof( ByteStream ),
                 "Please add member variables to the ByteStream base, not the ByteStream Reader." );

  // 把ByteStream*转化成Reader* 这样就拿到了接口
  return static_cast<Reader&>( *this ); // NOLINT(*-downcast)
}

// 和上面一样，只不过第一返回一个只读的接口引用(前面的const)
// 第二这个Reader接口不能改变ByteStream的成员变量(后面的const)
const Reader& ByteStream::reader() const
{
  static_assert( sizeof( Reader ) == sizeof( ByteStream ),
                 "Please add member variables to the ByteStream base, not the ByteStream Reader." );

  return static_cast<const Reader&>( *this ); // NOLINT(*-downcast)
}

// 同上
Writer& ByteStream::writer()
{
  static_assert( sizeof( Writer ) == sizeof( ByteStream ),
                 "Please add member variables to the ByteStream base, not the ByteStream Writer." );

  return static_cast<Writer&>( *this ); // NOLINT(*-downcast)
}

// 同上
const Writer& ByteStream::writer() const
{
  static_assert( sizeof( Writer ) == sizeof( ByteStream ),
                 "Please add member variables to the ByteStream base, not the ByteStream Writer." );

  return static_cast<const Writer&>( *this ); // NOLINT(*-downcast)
}

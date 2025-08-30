#include "byte_stream.hh"

using namespace std;

// ByteStream的构造实现

// ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}
// 成员变量的初始化，不然通不过编译
ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), error_( false ), buffer_(), bytes_pushed_( 0 ), bytes_popped_( 0 ), is_closed_( false )
{}

// 向缓冲区注入string
void Writer::push( string data )
{
  // (void)data; // Your code here.

  // 如果流已经关闭，或者出错，直接return
  if ( this->is_closed() || this->has_error() ) {
    return;
  }

  // 获取流的缓冲区还剩多少容量
  uint64_t available = this->available_capacity();

  // 如果缓冲区已经满了，直接return
  if ( available == 0 ) {
    return;
  }

  // 获取可注入的字节数，至多把data全部注入
  uint64_t length = std::min( data.size(), available );

  // 直接接到缓冲区的string后面
  this->buffer_.append( data.begin(), data.begin() + length );

  // 更新已注入的字节数
  this->bytes_pushed_ += length;
}

// 关闭流
void Writer::close()
{
  // Your code here.
  // 直接设置成员变量为关闭
  this->is_closed_ = true;
}

// 判断流是否关闭
bool Writer::is_closed() const
{
  // return {}; // Your code here.
  // 通过成员变量来判断是否关闭
  if ( this->is_closed_ == true ) {
    return true;
  } else {
    return false;
  }
}

// 获取流的缓冲区还能写下多少字节
uint64_t Writer::available_capacity() const
{
  // return {}; // Your code here.
  // 总注入-总弹出=目前仍在里面的
  uint64_t buffered = this->bytes_pushed_ - this->bytes_popped_;

  // 总容量-目前仍在里面的=可用空间
  uint64_t available = this->capacity_ - buffered;
  return available;
}

// 获取总共注入过多少
uint64_t Writer::bytes_pushed() const
{
  // return {}; // Your code here.
  return this->bytes_pushed_;
}

// 查看缓冲区里的string,但是是以非拥有性质的
string_view Reader::peek() const
{
  // return {}; // Your code here.
  return std::string_view( this->buffer_ );
}

// 弹出缓冲区内的n个字节
void Reader::pop( uint64_t len )
{
  // (void)len; // Your code here.
  // 如果流已经结束了或者出错了，直接return
  if ( this->is_finished() || this->has_error() ) {
    return;
  }

  // 获取弹出的字节数(最多全弹出)
  uint64_t length = std::min( len, this->buffer_.size() );

  // 缓冲区string直接erase掉length个字节
  this->buffer_.erase( 0, length );

  // 更新出栈的总字节数
  this->bytes_popped_ += length;
  // this->bytes_pushed_-=length;

  return;
}

// 判断流是否结束
bool Reader::is_finished() const
{
  // return {}; // Your code here.
  return this->is_closed_ && this->buffer_.empty();
}

// 获取缓冲区内字节数
uint64_t Reader::bytes_buffered() const
{
  // return {}; // Your code here.
  return this->buffer_.size();
}

// 获取总弹出的字节数
uint64_t Reader::bytes_popped() const
{
  // return {}; // Your code here.
  return this->bytes_popped_;
}
#include "byte_stream.hh"

using namespace std;

// ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}
ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), error_( false ), buffer_(), bytes_pushed_( 0 ), bytes_popped_( 0 ), is_closed_( false )
{}
void Writer::push( string data )
{
  // (void)data; // Your code here.
  if ( this->is_closed() || this->has_error() ) {
    return;
  }

  uint64_t available = this->available_capacity();

  if ( available == 0 ) {
    return;
  }

  uint64_t length = std::min( data.size(), available );

  this->buffer_.append( data.begin(), data.begin() + length );

  bytes_pushed_ += length;
}

void Writer::close()
{
  // Your code here.
  this->is_closed_ = true;
}

bool Writer::is_closed() const
{
  // return {}; // Your code here.
  if ( this->is_closed_ == true ) {
    return true;
  } else {
    return false;
  }
}

uint64_t Writer::available_capacity() const
{
  // return {}; // Your code here.
  uint64_t buffered = this->bytes_pushed_ - this->bytes_popped_;
  uint64_t available = this->capacity_ - buffered;
  return available;
}

uint64_t Writer::bytes_pushed() const
{
  // return {}; // Your code here.
  return this->bytes_pushed_;
}

string_view Reader::peek() const
{
  // return {}; // Your code here.
  return std::string_view( this->buffer_ );
}

void Reader::pop( uint64_t len )
{
  // (void)len; // Your code here.
  if ( this->is_finished() || this->has_error() ) {
    return;
  }

  uint64_t length = std::min( len, this->buffer_.size() );

  this->buffer_.erase( 0, length );
  this->bytes_popped_ += length;
  // this->bytes_pushed_-=length;

  return;
}

bool Reader::is_finished() const
{
  // return {}; // Your code here.
  return this->is_closed_ && this->buffer_.empty();
}

uint64_t Reader::bytes_buffered() const
{
  // return {}; // Your code here.
  return this->buffer_.size();
}

uint64_t Reader::bytes_popped() const
{
  // return {}; // Your code here.
  return this->bytes_popped_;
}
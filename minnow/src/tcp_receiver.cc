#include "tcp_receiver.hh"
#include "debug.hh"
#include <cstdlib>
#include <algorithm>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  
  // Your code here.
  // debug( "unimplemented receive() called" );
  // (void)message;

  if (message.RST)
  {
    this->reassembler_.reader().set_error();
    return;
  }

  if (message.SYN==true && !this->syn_received_)
  {
    this->isn_=message.seqno;
    this->syn_received_=true;
    // 初始序列号

    this->reassembler_.insert(0,"",false);
    this->reassembler_.increment_next_index();

    if (!message.payload.empty())
    {
      if (message.FIN)
      {
        reassembler_.insert(1,message.payload,true);
      }
      else
      {
        reassembler_.insert(1,message.payload,false);
      }
    }

    else if (message.FIN)
    {
      reassembler_.insert(1,"",true);
    }
    return;
  }

  if (this->isn_.has_value())
  {
    uint64_t checkpoint=reassembler_.writer().bytes_pushed();
    int64_t signed_index=static_cast<int64_t>(message.seqno.unwrap(*isn_, checkpoint)) - (message.SYN ? 1 : 0);
    uint64_t stream_index=(signed_index<0)?0:static_cast<uint64_t>(signed_index);

    reassembler_.insert(stream_index,message.payload,message.FIN);
    // if (message.SYN && message.FIN && message.payload.empty())
    // {
    //   if (!reassembler_.writer().is_closed())
    //   {

    //   }
    // }
  }



}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  // debug( "unimplemented send() called" );
  // return {};

  TCPReceiverMessage message;

  if (this->reassembler_.reader().has_error())
  {
    message.RST=true;;
    return message;
  }

  uint64_t capacity=this->reassembler_.writer().available_capacity();
  message.window_size=std::min(capacity,static_cast<uint64_t>(UINT16_MAX));

  if (this->isn_.has_value())
  {
    uint64_t bytes_pushed=this->reassembler_.writer().bytes_pushed();
    uint64_t absolute_ackno=bytes_pushed>0? bytes_pushed+1:1;

    
    if (this->reassembler_.reader().is_finished() | this->reassembler_.writer().is_closed())
    {
      absolute_ackno+=1;
    }

    message.ackno=Wrap32::wrap(absolute_ackno,*isn_);
  }

  return message;
}

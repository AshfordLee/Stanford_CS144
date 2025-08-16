#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // debug( "unimplemented sequence_numbers_in_flight() called" );
  // return {};
  return this->bytes_in_flight;

}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  // debug( "unimplemented consecutive_retransmissions() called" );
  // return {};
  return this->consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // debug( "unimplemented push() called" );
  // (void)transmit;
  uint64_t effective_window_size=(this->window_size==0)?1:this->window_size;

  if (this->fin_sent)
  {
    return;
  }

  if (this->input_.reader().has_error())
  {
    TCPSenderMessage msg;
    msg.seqno=Wrap32::wrap(this->next_seqno,this->isn_);
    msg.RST=true;

    transmit(msg);
    return;
  }

  if (!this->syn_sent && this->bytes_in_flight<effective_window_size)
  {
    TCPSenderMessage msg;
    msg.SYN=true;
    msg.seqno=Wrap32::wrap(this->next_seqno,this->isn_);

    // bool has_data=false;

    if (this->input_.reader().bytes_buffered()>0)
    {
      uint64_t flags_space=1;

      if (this->input_.reader().is_finished() && !this->fin_sent)
      {
        flags_space+=1;
      }

      uint64_t available_window=effective_window_size-bytes_in_flight;
      uint64_t max_payload=available_window-flags_space;
      uint64_t payload_size=min(max_payload,TCPConfig::MAX_PAYLOAD_SIZE);

      msg.payload=this->input_.reader().peek().substr(0,payload_size);
      this->input_.reader().pop(payload_size);
      // has_data=true;
    }
    this->syn_sent=true;

    // if (this->input_.reader().is_finished()  && !this->fin_sent && !has_data)
    if (this->input_.reader().is_finished()  && !this->fin_sent)
    {
      msg.FIN=true;
      this->fin_sent=true;
    }

    OutstandingSegment segment{
      .msg = msg,
      .seqno = this->next_seqno,
      .length = msg.sequence_length(),
      .transmit_time = 0
    };

    this->outstanding_segments.push_back(segment);
    this->bytes_in_flight+=msg.sequence_length();
    this->next_seqno+=msg.sequence_length();

    transmit(msg);

    if(!this->time_running)
    {
      this->time_running=true;
      this->consecutive_retransmissions_=0;
    }

  }

  while( this->bytes_in_flight<effective_window_size  && 
  input_.reader().bytes_buffered() > 0)
  {
    TCPSenderMessage msg;

    msg.seqno=Wrap32::wrap(this->next_seqno,this->isn_);

    uint64_t available_window=effective_window_size-bytes_in_flight;
    uint64_t payload_size=min(available_window,TCPConfig::MAX_PAYLOAD_SIZE);

    msg.payload=this->input_.reader().peek().substr(0,payload_size);
    this->input_.reader().pop(payload_size);

    if(this->input_.reader().is_finished() && !this->fin_sent && this->bytes_in_flight+msg.sequence_length()+1<=effective_window_size)
    {
      msg.FIN=true;
      this->fin_sent=true;
    }

    OutstandingSegment segment{
      .msg=msg,
      .seqno=this->next_seqno,
      .length=msg.sequence_length(),
      .transmit_time=0
    };

    this->outstanding_segments.push_back(segment);
    this->bytes_in_flight+=msg.sequence_length();
    this->next_seqno+=msg.sequence_length();

    transmit(msg);

    if(!this->time_running)
    {
      this->time_running=true;
      this->consecutive_retransmissions_=0;
    }

  }

  if(input_.reader().is_finished() && this->bytes_in_flight<effective_window_size && !this->fin_sent)
  {
    TCPSenderMessage msg;

    msg.FIN=true;
    this->fin_sent=true;
    msg.seqno=Wrap32::wrap(this->next_seqno,this->isn_);

  OutstandingSegment segment{
    .msg=msg,
    .seqno=this->next_seqno,
    .length=msg.sequence_length(),
    .transmit_time=0
  };

  outstanding_segments.push_back(segment);

  this->bytes_in_flight+=msg.sequence_length();
  this->next_seqno+=msg.sequence_length();

  transmit(msg);

  if(!this->time_running)
  {
    this->time_running=true;
    this->consecutive_retransmissions_=0;
  }
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // debug( "unimplemented make_empty_message() called" );
  // return {};
  TCPSenderMessage Message;
  Message.seqno=Wrap32::wrap(this->next_seqno,this->isn_);
  Message.FIN=false;
  Message.SYN=false;
  Message.RST=this->input_.has_error();
  Message.payload="";

  return Message;

}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // debug( "unimplemented receive() called" );
  // (void)msg;
  this->window_size=msg.window_size;

  if (!msg.ackno.has_value() && msg.window_size==0)
  {
    this->input_.writer().set_error();
    return;
  }

  if (!msg.ackno.has_value())
  {
    // this->input_.set_error();
    return;
  }

  if (msg.RST)
  {
    this->outstanding_segments.clear();
    this->bytes_in_flight=0;
    this->time_running=false;
    return;
  }

  this->window_size=msg.window_size;

  uint64_t abs_ackno=msg.ackno.value().unwrap(this->isn_,this->next_seqno);

  if (abs_ackno>this->next_seqno)
  {
    return;
  }

  bool segments_acked=false;
  for (auto it=this->outstanding_segments.begin();it!=this->outstanding_segments.end();)
  {
    if (abs_ackno>=it->seqno+it->length)
    {
      segments_acked=true;
      this->bytes_in_flight-=it->length;
      it=this->outstanding_segments.erase(it);
    }
    else
    {
      ++it;
    }
  }

  if (segments_acked)
  {
    this->consecutive_retransmissions_=0;
    this->current_RTO_ms_=this->initial_RTO_ms_;
  }

}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // debug( "unimplemented tick({}, ...) called", ms_since_last_tick );
  // (void)transmit;
  if (!this->time_running)
  {
    return;
  }

  if (!this->outstanding_segments.empty())
  {
    this->outstanding_segments.front().transmit_time+=ms_since_last_tick;

    if(this->outstanding_segments.front().transmit_time>=this->current_RTO_ms_)
    {
      transmit(this->outstanding_segments.front().msg);

      this->outstanding_segments.front().transmit_time=0;

      this->consecutive_retransmissions_++;

      if (this->window_size>0)
      {
      this->current_RTO_ms_*=2;
      }
    }
  }
}



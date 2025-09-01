#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // debug( "unimplemented send_datagram called" );
  // (void)dgram;
  // (void)next_hop;
  uint32_t next_hop_ip=next_hop.ipv4_numeric();

  auto cache_it=this->arp_cache_.find(next_hop_ip);

  if(cache_it!=this->arp_cache_.end())
  {
    if(this->timestamp_ms_-cache_it->second.timestamp<=30000)
    {
    this->send_frame_immediately(dgram,cache_it->second.ethernet_address);
    return;
    }

    else
    {
      this->arp_cache_.erase(cache_it);
    }
  }

  bool should_send_arp=true;

  if(this->arp_timer_.find(next_hop_ip)!=this->arp_timer_.end())
  {
    if(this->timestamp_ms_-this->arp_timer_[next_hop_ip]<5000)
    {
      should_send_arp=false;
    }
  }

  if (should_send_arp)
  {
    send_arp_request(next_hop_ip);
    this->arp_timer_[next_hop_ip]=this->timestamp_ms_;
  }

  this->arp_pending_queue_[next_hop_ip].push(dgram);
}

void NetworkInterface::send_frame_immediately(const InternetDatagram& dgram,const EthernetAddress& dst_ethernet_address)
{
  EthernetFrame frame;
  frame.header.type=EthernetHeader::TYPE_IPv4;
  frame.header.dst=dst_ethernet_address;
  frame.header.src=this->ethernet_address_;

  Serializer serializer;
  dgram.serialize(serializer);

  std::vector<Ref<std::string>> serialized_data=serializer.finish();

  frame.payload=serialized_data;

  transmit(frame);
}



void NetworkInterface::send_arp_request(const uint32_t target_ip)
{
  EthernetFrame frame;
  frame.header.type=EthernetHeader::TYPE_ARP;
  frame.header.dst=ETHERNET_BROADCAST;
  frame.header.src=this->ethernet_address_;

  ARPMessage arp_request;
  arp_request.opcode=ARPMessage::OPCODE_REQUEST;
  arp_request.sender_ethernet_address=this->ethernet_address_;
  arp_request.sender_ip_address=this->ip_address_.ipv4_numeric();
  arp_request.target_ip_address=target_ip;
  arp_request.target_ethernet_address={0,0,0,0,0,0};

  Serializer serializer;
  arp_request.serialize(serializer);
  frame.payload=serializer.finish();

  transmit(frame);
}

void NetworkInterface::send_arp_reply(const uint32_t target_ip,const EthernetAddress& target_ethernet_address)
{
  EthernetFrame frame;
  frame.header.type=EthernetHeader::TYPE_ARP;
  frame.header.dst=target_ethernet_address;
  frame.header.src=this->ethernet_address_;

  ARPMessage arp_reply;
  arp_reply.opcode=ARPMessage::OPCODE_REPLY;
  arp_reply.sender_ethernet_address=this->ethernet_address_;
  arp_reply.sender_ip_address=this->ip_address_.ipv4_numeric();
  arp_reply.target_ip_address=target_ip;
  arp_reply.target_ethernet_address=target_ethernet_address;

  Serializer serializer;
  arp_reply.serialize(serializer);
  frame.payload=serializer.finish();

  transmit(frame);

}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( EthernetFrame frame )
{
  // debug( "unimplemented recv_frame called" );
  // (void)frame;
  if (frame.header.dst!=this->ethernet_address_ && frame.header.dst!=ETHERNET_BROADCAST)
  {
    return;
  }

  if(frame.header.type==EthernetHeader::TYPE_IPv4)
  {
    InternetDatagram dgram;

    if (parse(dgram,frame.payload))
    {
      this->datagrams_received_.push(dgram);
    }
    return;
  }
  if(frame.header.type==EthernetHeader::TYPE_ARP)
  {
    ARPMessage arp_msg;
    if (parse(arp_msg,frame.payload))
    {
      this->arp_cache_[arp_msg.sender_ip_address]=ARPCacheEntry(arp_msg.sender_ethernet_address,this->timestamp_ms_);

      if (arp_msg.opcode==ARPMessage::OPCODE_REQUEST && arp_msg.target_ip_address==this->ip_address_.ipv4_numeric())
      {
        send_arp_reply(arp_msg.sender_ip_address,arp_msg.sender_ethernet_address);
      }

      uint32_t sender_ip=arp_msg.sender_ip_address;
      if (this->arp_pending_queue_.find(sender_ip)!=this->arp_pending_queue_.end())
      {
        auto& queue=this->arp_pending_queue_[sender_ip];
        while(!queue.empty())
        {
          send_frame_immediately(queue.front(),arp_msg.sender_ethernet_address);
          queue.pop();
        }
        this->arp_pending_queue_.erase(sender_ip);
      }

      if (arp_msg.opcode==ARPMessage::OPCODE_REPLY)
      {
        if (this->arp_pending_queue_.find(sender_ip)!=this->arp_pending_queue_.end())
        {
          auto& queue=this->arp_pending_queue_[sender_ip];
          while(!queue.empty())
          {
            send_frame_immediately(queue.front(),arp_msg.sender_ethernet_address);
            queue.pop();
          }
          this->arp_pending_queue_.erase(sender_ip);
        }
      }
    }
  }
}


//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // debug( "unimplemented tick({}) called", ms_since_last_tick );
  this->timestamp_ms_+=ms_since_last_tick;
  auto it=this->arp_cache_.begin();
  while(it!=this->arp_cache_.end())
  {
    if(this->timestamp_ms_-it->second.timestamp>30000)
    {
      it=this->arp_cache_.erase(it);
    }
    else
    {
      ++it;
    }
  }

  auto queue_it = this->arp_pending_queue_.begin();
  while (queue_it != this->arp_pending_queue_.end()) {
    uint32_t ip = queue_it->first;
    // 如果ARP请求已过期（超过5秒），丢弃待发送的数据包
    if (this->arp_timer_.find(ip) != this->arp_timer_.end() && 
        this->timestamp_ms_ - this->arp_timer_[ip] > 5000) {
      queue_it = this->arp_pending_queue_.erase(queue_it);
    } else {
      ++queue_it;
    }
  }
}

#include "router.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // debug( "unimplemented add_route() called" );
  this->routing_table_.push_back(RouteEntry(route_prefix,prefix_length,next_hop,interface_num));
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // debug( "unimplemented route() called" );
  for (auto& interface:this->interfaces_)
  {
    while(!interface->datagrams_received().empty())
    {
      auto datagram=interface->datagrams_received().front();
      interface->datagrams_received().pop();
      
      uint32_t destination=datagram.header.dst;
      cerr << "DEBUG: Processing datagram to " << Address::from_ipv4_numeric(destination).ip() << endl;
      std::optional<RouteEntry> best_route=this->find_best_route(destination);
      
      if (datagram.header.ttl<=1)
      {
        cerr << "DEBUG: TTL <= 1, dropping datagram" << endl;
        continue;
      }



      if (best_route.has_value())
      {
        cerr << "DEBUG: Found route, forwarding datagram" << endl;
        datagram.header.ttl--;

        datagram.header.compute_checksum();
        forward_datagram(datagram,best_route.value());
      }
      else
      {
        cerr << "DEBUG: No route found, dropping datagram" << endl;
      }
    }
  }
}


std::optional<Router::RouteEntry> Router::find_best_route(uint32_t destination)
{
  Router::RouteEntry* best_match=nullptr;
  uint8_t best_match_length=0;

  cerr << "DEBUG: Looking for route to " << Address::from_ipv4_numeric(destination).ip() << endl;

  for (auto& route:this->routing_table_)
  {
    uint8_t prefix_length=route.prefix_length;
    uint32_t route_prefix=route.route_prefix;

    uint32_t mask;

    if (prefix_length==0)
    {
      mask=0x00000000;
    }
    else
    {
      mask=0xFFFFFFFF<<(32-prefix_length);
    }

    bool matches = (destination & mask)==(route_prefix & mask);
    
    cerr << "DEBUG: Checking route " << Address::from_ipv4_numeric(route_prefix).ip() 
         << "/" << static_cast<int>(prefix_length) 
         << " -> " << (route.next_hop.has_value() ? route.next_hop->ip() : "(direct)")
         << " on interface " << route.interface_num
         << " (matches: " << (matches ? "YES" : "NO") << ")" << endl;

    if (matches)
    {
      if (prefix_length>=best_match_length)
      {
        best_match_length=prefix_length;
        best_match=&route;
        cerr << "DEBUG: New best match found!" << endl;
      }
    }

  }

  if (best_match!=nullptr)
  {
    cerr << "DEBUG: Final best route: " << Address::from_ipv4_numeric(best_match->route_prefix).ip()
    << "/" << static_cast<int>(best_match->prefix_length)
    << " -> " << (best_match->next_hop.has_value() ? best_match->next_hop->ip() : "(direct)")
    << " on interface " << best_match->interface_num << endl;
    return *best_match;
  }

  cerr << "DEBUG: No route found!" << endl;
  return std::nullopt;
}

void Router::forward_datagram(InternetDatagram& datagram,const RouteEntry& route)
{


  Address next_hop=route.next_hop.value_or(Address::from_ipv4_numeric(datagram.header.dst));

  cerr << "DEBUG: Forwarding datagram to " << next_hop.ip() 
  << " via interface " << route.interface_num << endl;

  interface(route.interface_num)->send_datagram(datagram,next_hop);
}
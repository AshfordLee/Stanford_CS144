#include "socket.hh"
#include <arpa/inet.h>
#include <iostream>
using namespace std;

class RawSocket : public DatagramSocket
{
public:
  RawSocket() : DatagramSocket( AF_INET, SOCK_RAW, IPPROTO_RAW ) {}
};

class IPv4Header
{
public:
  uint8_t version_ihl = 0x45;
  // 4代表版本号，IPv4的这里固定为4
  // 5代表IP报文头的长度，这里是5*4=20个字节的报文头
  uint8_t tos = 0;
  // tos表示服务类型，由以下几个东西组成
  // 优先级Precedence(3 bit) 比如001代表Priority等等
  // 延迟Delay(1 bit) 0代表低延迟 1代表最小化延迟
  // 吞吐量Throughput(1 bit) 0普通吞吐 1最大化吞吐
  // 可靠性Reliability(1 bit) 0代表不可靠 1代表可靠
  // 成本Cost(1 bit) 0代表低成本 1代表高成本
  // 保留位Reserved(1 bit)
  // 总共8个bit
  uint16_t total_length = 0;
  // 表示整个数据包(IP报头+数据)总共是多大
  uint16_t id = htons( 1234 );
  // 分片的时候，会分配id，这样重组的时候可以完成重组
  uint16_t flags_frag = htons( 0x4000 );
  // 指示是否分片和分片信息
  // Flags(3 bit) 位15是保留 位14是DF(Don't Fragment) 0代表允许分片 1代表禁止分片
  // 禁止分片的意思就是如果报文长度超过MTU就会被直接丢弃
  // 位13是MF(More Fragment) 0代表最后一个分片 1代表还有更多分片
  // 低13位是分片偏移量 例如如果低13位是100 那么分片数据从原始报文的100*8字节处开始
  uint8_t ttl = 64;
  // 生存时间 允许最多通过的路由(跳)数
  uint8_t protocol = 0;
  // 指示上层协议模块 例如1代表ICMP 6代表TCP
  uint16_t checksum = 0;
  // 校验和 用于验证IP报头和数据是否被损坏
  uint32_t src_addr = 0;
  // 源IP地址
  uint32_t dst_addr = 0;
  // 目的IP地址

  void compute_checksum()
  {
    this->checksum = 0;
    uint32_t sum = 0;
    const uint16_t* ptr = reinterpret_cast<const uint16_t*>( this );
    // 把指向this(IPv4Header类)的指针转化为指向16位数的指针

    for ( size_t i = 0; i < sizeof( *this ) / 2; i++ )
    // sizeof(*this)是"这个类里面有多少个8位数"，除以2是因为每个16位数是2个8位数
    {
      sum += ntohs( ptr[i] );
      // 求和
    }

    while ( sum >> 16 ) {
      sum = ( sum & 0xFFFF ) + ( sum >> 16 );
    }

    this->checksum = htons( ~sum );
  }
};

class UDPHeader
{
public:
  uint16_t src_port = 0;
  // 源端口
  uint16_t dst_port = 0;
  // 目的端口
  uint16_t length = 0;
  // 长度
  uint16_t checksum = 0;
  // 校验和
};

int main()
{
  // construct an Internet or user datagram here, and send using the RawSocket as in the Jan. 10 lecture
  RawSocket raw_socket;
  std::string target_ip = "106.39.200.249";
  Address target( target_ip );

  std::cout << "发送数据报到" << target_ip << std::endl;

  std::string payload5 = "This is a test message for Protocal 5";

  IPv4Header ip_header5;
  ip_header5.protocol = 5;
  ip_header5.src_addr = inet_addr( "192.168.71.40" );
  ip_header5.dst_addr = inet_addr( target_ip.c_str() );
  ip_header5.total_length = htons( sizeof( IPv4Header ) + payload5.size() );
  ip_header5.compute_checksum();

  std::string datagram5;
  datagram5.append( reinterpret_cast<char*>( &ip_header5 ), sizeof( ip_header5 ) );
  datagram5.append( payload5 );

  std::cout << "Start sending datagram of protocal 5" << std::endl;
  raw_socket.sendto( target, datagram5 );
  std::cout << "Sent datagram of protocal 5 Complete" << std::endl;

  std::cout << "Start sending datagram of protocal 17" << std::endl;

  std::string payload17 = "This is a test message for Protocal 17";

  UDPHeader udp_header;
  udp_header.src_port = htons( 12345 );                                // 源端口，可以是任意值
  udp_header.dst_port = htons( 8080 );                                 // 目标端口，需与接收方匹配
  udp_header.length = htons( sizeof( UDPHeader ) + payload17.size() ); // UDP总长度
  udp_header.checksum = 0;                                             // UDP校验和可以为0（可选）

  IPv4Header ip_header17;
  ip_header17.protocol = 17;                           // 设置协议为UDP(17)
  ip_header17.src_addr = inet_addr( "192.168.71.40" ); // 修改为你的IP
  ip_header17.dst_addr = inet_addr( target_ip.c_str() );
  ip_header17.total_length = htons( sizeof( IPv4Header ) + sizeof( UDPHeader ) + payload17.size() );
  ip_header17.compute_checksum(); // 计算IP头部校验和

  std::string udp_datagram;
  udp_datagram.append( reinterpret_cast<const char*>( &ip_header17 ), sizeof( ip_header17 ) );
  udp_datagram.append( reinterpret_cast<const char*>( &udp_header ), sizeof( udp_header ) );
  udp_datagram.append( payload17 );

  std::cout << "Start sending datagram of protocal 17" << std::endl;
  raw_socket.sendto( target, udp_datagram );
  std::cout << "Sent datagram of protocal 17 Complete" << std::endl;

  return 0;
}

#include "wrapping_integers.hh"
#include "debug.hh"
#include <stdio.h>
#include <cstdlib>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  uint32_t result=n+zero_point.raw_value_;
  return Wrap32 { result };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // debug( "unimplemented unwrap( {}, {} ) called", zero_point.raw_value_, checkpoint );
  // 计算相对于zero_point的偏移量
  uint32_t offset = this->raw_value_ - zero_point.raw_value_;
  uint64_t t = (checkpoint & 0xFFFFFFFF00000000) + offset;
  uint64_t ret = t;
  if (abs(int64_t(t + (1ul << 32) - checkpoint)) < abs(int64_t(t - checkpoint)))
      ret = t + (1ul << 32);
  if (t >= (1ul << 32) && abs(int64_t(t - (1ul << 32) - checkpoint)) < abs(int64_t(ret - checkpoint)))
      ret = t - (1ul << 32);
  return ret;
}

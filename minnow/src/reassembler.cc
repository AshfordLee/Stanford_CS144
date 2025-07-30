#include "reassembler.hh"
#include "debug.hh"
#include <vector>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{

  if ( is_last_substring )
  // 当收到最后一个包的时候，就可以断言eof_index_是什么了
  {
    eof_flag_ = true;
    uint64_t available_capacity = output_.writer().available_capacity();
    uint64_t original_size = data.size();
    // 注意后面可能要截断数据，但是eof_index_是根据原始数据计算的
    if ( first_index > next_index_ )
    // 最后一个段 且要写入缓冲区
    {
      uint64_t index_gap = first_index - next_index_;
      // 这是索引差距，也就是一定要预留的空间
      if ( index_gap + data.size() > available_capacity ) {
        // 这里的意思就是预留的空间+现在需要的空间>可用空间
        // 就要截断了

        uint64_t can_store = ( available_capacity > index_gap ) ? ( available_capacity - index_gap ) : 0;
        // 想想 只有在available_capacity比预留空间大的时候
        // 我才有可能拿出空间写入一部分data
        data = data.substr( 0, can_store );
      }
    }
    eof_index_ = first_index + original_size;
  }

  if ( first_index == next_index_ ) {
    push_output( data );
    try_assemble();
    // 最好的情况，包按顺序来，直接写入
  } else if ( first_index < next_index_ )
  // 来的包比期望的更早(index更小)
  {
    if ( first_index + data.size() <= next_index_ ) {
      // 来的包全部都处理过
      return;
    } else {
      // 来的包有一些没处理过
      std::string partial_data = data.substr( next_index_ - first_index );
      push_output( partial_data );
      try_assemble();
    }
  } else if ( first_index > next_index_ ) {
    // 来的包index更大，现在还没法处理
    store_segment( first_index, data );
  }

  if ( next_index_ == eof_index_ && eof_flag_ == true ) {
    output_.writer().close();
    return;
  }
}

void Reassembler::try_assemble()
{
  // 提高健壮性，处理一种比较特殊的情况
  // 比如现在it->first==9 next_index==10
  // 那其实有可能it的后面那部分(从10开始的那段)应该是要拼接上去的
  while ( true ) {
    auto it = unassembled_segments_.begin();
    while ( it != unassembled_segments_.end() && it->first > next_index_ ) {
      it++;
    }

    if ( it != unassembled_segments_.end() && it->first <= next_index_ ) {
      uint64_t offset = ( next_index_ > it->first ) ? ( next_index_ - it->first ) : 0;
      if ( offset >= it->second.size() ) {
        bytes_pending_ -= it->second.size();
        unassembled_segments_.erase( it );
      } else {
        std::string data = it->second.substr( offset );
        push_output( data );
        bytes_pending_ = bytes_pending_ - it->second.size();
        unassembled_segments_.erase( it );
      }
    }

    else {
      break;
    }
  }
}
// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  return bytes_pending_;
}

void Reassembler::push_output( std::string& data )
{
  // 把data写入output
  // 注意当我们想把一个data写入output的时候
  // 其实不一定写的进去 因为要考虑output的capacity

  uint64_t available_capacity = output_.writer().available_capacity();

  if ( available_capacity == 0 ) {
    return;
  }

  if ( data.size() <= available_capacity ) {
    // 皆大欢喜
    output_.writer().push( data );
    next_index_ = next_index_ + data.size();
  }

  else {
    // 写不进去(capacity不够了)
    std::string partial_data = data.substr( 0, available_capacity );
    output_.writer().push( partial_data );
    std::string remaining_data = data.substr( available_capacity );
    next_index_ = next_index_ + partial_data.size();
    // 剩下来的还要存起来
    // store_segment(first_index+available_capacity,remaining_data);
  }
}

void Reassembler::store_segment( uint64_t first_index, std::string& data )
{
  // 检查 如果first_index巨大，根本就不需要储存，直接return
  // 所谓"巨大"，就是预留区已经比capacity大了
  // 那这时候就没办法再插入data了
  // 因为预留区是要被优先保证的
  if ( first_index - next_index_ > output_.writer().available_capacity() ) {
    return;
  }

  // 处理新老段有overlap的场合，这里比较复杂
  uint64_t available_capacity = output_.writer().available_capacity();
  uint64_t last_index = first_index + data.size() - 1;

  std::vector<uint64_t> overlapping_indices;

  for ( const auto& segment : unassembled_segments_ ) {
    uint64_t seg_first = segment.first;
    uint64_t seg_last = seg_first + segment.second.size() - 1;

    if ( seg_first <= last_index && first_index <= seg_last )
    // 有重复
    {
      overlapping_indices.push_back( seg_first );
    }
  }

  if ( overlapping_indices.empty() ) {
    // 无重复
    uint64_t index_gap = ( first_index - next_index_ > 0 ) ? ( first_index - next_index_ ) : 0;
    uint64_t remaining_capacity = ( available_capacity > index_gap ) ? ( available_capacity - index_gap ) : 0;

    // 检查剩余容量是否足够存储整个数据
    if ( data.size() > remaining_capacity ) {
      // 只能存储部分数据
      if ( remaining_capacity == 0 ) {
        return;
      }

      std::string partial_data = data.substr( 0, remaining_capacity );
      bytes_pending_ += partial_data.size();
      unassembled_segments_[first_index] = partial_data;
      return;
    }

    // 能全部存下
    bytes_pending_ += data.size();
    unassembled_segments_[first_index] = data;
    return;
  }
  // 有重复
  for ( uint64_t idx : overlapping_indices ) {
    // 把老段拿出来
    std::string& old_data = unassembled_segments_[idx];
    uint64_t old_first = idx;
    uint64_t old_last = old_first + old_data.size() - 1;

    // 第一种情况:新段是老段的子集，那就不需要做任何事情
    if ( first_index >= old_first && last_index <= old_last ) {
      return;
    }

    // 第二种情况，老段是新段的子集，那删除老段
    if ( first_index < old_first && last_index > old_last ) {
      bytes_pending_ -= old_data.size();
      unassembled_segments_.erase( idx );
      continue;
    }

    // 第三种情况，两个段有overlap
    // 这种情况要合成一个大段
    std::string merged_data;
    uint64_t merged_first;

    if ( first_index < old_first ) {
      // 新段--OVERLAP--老段
      merged_first = first_index;
      merged_data = data;

      // 注意还要补上老段的不overlap的部分
      uint64_t overlap_size = last_index - old_first + 1;
      if ( overlap_size < old_data.size() ) {
        merged_data += old_data.substr( overlap_size );
      }
    }

    else {
      // 老段--OVERLAP--新段
      merged_first = old_first;
      merged_data = old_data;

      uint64_t overlap_size = old_last - first_index + 1;
      if ( overlap_size < data.size() ) {
        merged_data += data.substr( overlap_size );
      }
    }

    data = merged_data;
    first_index = merged_first;
    last_index = first_index + merged_data.size() - 1;

    // 删除老段
    bytes_pending_ -= old_data.size();
    unassembled_segments_.erase( idx );
  }

  // 从这个循环里出来之后，应该已经得到了最大的merge_data
  if ( data.size() + bytes_pending_ > available_capacity ) {
    // 和非重叠情况一样，尝试存储部分数据
    uint64_t index_gap = ( first_index - next_index_ > 0 ) ? ( first_index - next_index_ ) : 0;
    uint64_t can_store = ( available_capacity > index_gap ) ? ( available_capacity - index_gap ) : 0;

    if ( can_store == 0 ) {
      return;
    }

    std::string partial_data = data.substr( 0, can_store );
    bytes_pending_ += partial_data.size();
    unassembled_segments_[first_index] = partial_data;
    return;
  }

  bytes_pending_ += data.size();
  unassembled_segments_[first_index] = data;

  // // 把不能立即处理的段存起来
  // auto it=unassembled_segments_.find(first_index);

  // if (it!=unassembled_segments_.end())
  // // 找到了，但是data比原来的大
  // // 注意要比较capacity
  // {
  //   if (data.size()>it->second.size())
  //   {
  //     if (data.size()+bytes_pending_>available_capacity)
  //     {
  //       return;
  //     }

  //     bytes_pending_=bytes_pending_-it->second.size()+data.size();
  //     it->second=data;
  //   }
  // }

  // else
  // {
  //   if (data.size()+bytes_pending_>available_capacity)
  //   {
  //     return;
  //   }

  //   bytes_pending_=bytes_pending_+data.size();
  //   unassembled_segments_[first_index]=data;
  // }
}

void Reassembler::increment_next_index()
{
  this->next_index_++;
}
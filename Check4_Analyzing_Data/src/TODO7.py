# start_aigc
import re
import datetime
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

def parse_ping_data(filename):
    """解析ping数据，提取时间和RTT"""
    times = []
    rtts = []
    
    with open(filename, 'r') as file:
        for line in file:
            # 匹配时间戳和RTT
            time_match = re.search(r'\[(\d+\.\d+)\]', line)
            rtt_match = re.search(r'time=(\d+\.?\d*) ms', line)
            
            if time_match and rtt_match:
                # 转换Unix时间戳为datetime对象
                timestamp = float(time_match.group(1))
                dt = datetime.datetime.fromtimestamp(timestamp)
                
                # 提取RTT值
                rtt = float(rtt_match.group(1))
                
                times.append(dt)
                rtts.append(rtt)
    
    return times, rtts

def create_rtt_graph(times, rtts):
    """创建RTT随时间变化的图表"""
    plt.figure(figsize=(12, 8))
    
    # 绘制散点图
    plt.plot(times, rtts, 'o-', markersize=4, linewidth=1, alpha=0.7)
    
    # 设置X轴格式
    plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
    plt.gca().xaxis.set_major_locator(mdates.MinuteLocator(interval=10))  # 每10分钟一个刻度
    
    # 设置标签
    plt.xlabel('Time of Day')
    plt.ylabel('RTT (milliseconds)')
    plt.title('RTT vs Time')
    
    # 旋转X轴标签
    plt.xticks(rotation=45)
    
    # 添加网格
    plt.grid(True, alpha=0.3)
    
    # 自动调整布局
    plt.tight_layout()
    
    # 保存图表
    plt.savefig('rtt_vs_time.png', dpi=300, bbox_inches='tight')
    plt.show()

def main():
    # 解析数据
    times, rtts = parse_ping_data('../data/baidu.txt')
    
    if not times:
        print("没有找到有效数据")
        return
    
    print(f"总共解析了 {len(times)} 个数据点")
    print(f"时间范围: {times[0]} 到 {times[-1]}")
    print(f"RTT范围: {min(rtts):.1f} ms 到 {max(rtts):.1f} ms")
    
    # 创建图表
    create_rtt_graph(times, rtts)
    
    # 输出一些统计信息
    print(f"\nRTT统计:")
    print(f"平均RTT: {sum(rtts)/len(rtts):.2f} ms")
    print(f"最小RTT: {min(rtts):.1f} ms")
    print(f"最大RTT: {max(rtts):.1f} ms")

if __name__ == "__main__":
    main()
# end_aigc
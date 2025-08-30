# start_aigc
import re
import numpy as np
import matplotlib.pyplot as plt
from scipy import stats

def parse_ping_data(filename):
    """解析ping数据，提取RTT"""
    rtts = []
    
    with open(filename, 'r') as file:
        for line in file:
            # 匹配RTT
            rtt_match = re.search(r'time=(\d+\.?\d*) ms', line)
            
            if rtt_match:
                rtt = float(rtt_match.group(1))
                rtts.append(rtt)
    
    return rtts

def create_cdf_graph(rtts):
    """创建RTT的累积分布函数图表"""
    # 对RTT值排序
    sorted_rtts = np.sort(rtts)
    
    # 计算CDF值
    n = len(sorted_rtts)
    cdf_values = np.arange(1, n + 1) / n
    
    # 创建图表
    plt.figure(figsize=(10, 8))
    
    # 绘制CDF
    plt.plot(sorted_rtts, cdf_values, 'b-', linewidth=2, label='Empirical CDF')
    
    # 设置坐标轴
    plt.xlabel('RTT (milliseconds)')
    plt.ylabel('Cumulative Probability')
    plt.title('Cumulative Distribution Function of RTT')
    
    # 设置Y轴范围
    plt.ylim(0, 1)
    
    # 添加网格
    plt.grid(True, alpha=0.3)
    
    # 添加一些统计信息
    mean_rtt = np.mean(rtts)
    median_rtt = np.median(rtts)
    std_rtt = np.std(rtts)
    
    # 在图表上标注关键统计值
    plt.axhline(y=0.5, color='r', linestyle='--', alpha=0.7, label=f'Median: {median_rtt:.1f} ms')
    plt.axhline(y=0.9, color='g', linestyle='--', alpha=0.7, label=f'90th percentile')
    plt.axhline(y=0.95, color='orange', linestyle='--', alpha=0.7, label=f'95th percentile')
    
    # 添加图例
    plt.legend()
    
    # 自动调整布局
    plt.tight_layout()
    
    # 保存图表
    plt.savefig('rtt_cdf.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    return sorted_rtts, cdf_values

def analyze_distribution_shape(rtts):
    """分析RTT分布的形状"""
    print("\n=== RTT分布形状分析 ===")
    
    # 基本统计
    mean_rtt = np.mean(rtts)
    median_rtt = np.median(rtts)
    std_rtt = np.std(rtts)
    skewness = stats.skew(rtts)
    kurtosis = stats.kurtosis(rtts)
    
    print(f"样本数量: {len(rtts)}")
    print(f"平均值: {mean_rtt:.2f} ms")
    print(f"中位数: {median_rtt:.2f} ms")
    print(f"标准差: {std_rtt:.2f} ms")
    print(f"偏度: {skewness:.3f}")
    print(f"峰度: {kurtosis:.3f}")
    
    # 分布形状判断
    print("\n分布形状分析:")
    
    if abs(skewness) < 0.5:
        print("- 分布相对对称")
    elif skewness > 0.5:
        print("- 分布右偏（长尾在右侧）")
    else:
        print("- 分布左偏（长尾在左侧）")
    
    if kurtosis > 3:
        print("- 分布尖峰（比正态分布更集中）")
    elif kurtosis < 3:
        print("- 分布平缓（比正态分布更分散）")
    else:
        print("- 分布接近正态分布")
    
    # 计算分位数
    percentiles = [25, 50, 75, 90, 95, 99]
    print(f"\n分位数:")
    for p in percentiles:
        value = np.percentile(rtts, p)
        print(f"  {p}th percentile: {value:.2f} ms")

def main():
    # 解析数据
    rtts = parse_ping_data('../data/baidu.txt')
    
    if not rtts:
        print("没有找到有效数据")
        return
    
    print(f"总共解析了 {len(rtts)} 个RTT数据点")
    print(f"RTT范围: {min(rtts):.1f} ms 到 {max(rtts):.1f} ms")
    
    # 创建CDF图表
    sorted_rtts, cdf_values = create_cdf_graph(rtts)
    
    # 分析分布形状
    analyze_distribution_shape(rtts)
    
    # 输出一些CDF关键值
    print(f"\n=== CDF关键值 ===")
    print(f"50%的ping包RTT ≤ {np.percentile(rtts, 50):.2f} ms")
    print(f"90%的ping包RTT ≤ {np.percentile(rtts, 90):.2f} ms")
    print(f"95%的ping包RTT ≤ {np.percentile(rtts, 95):.2f} ms")
    print(f"99%的ping包RTT ≤ {np.percentile(rtts, 99):.2f} ms")

if __name__ == "__main__":
    main()
# end_aigc
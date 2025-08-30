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

def create_correlation_scatter(rtts):
    """创建相邻RTT的相关性散点图"""
    if len(rtts) < 2:
        print("数据点不足，无法创建相关性图")
        return None, None
    
    # 创建相邻RTT对
    rtt_n = rtts[:-1]  # 第N个RTT
    rtt_n_plus_1 = rtts[1:]  # 第N+1个RTT
    
    # 计算相关系数
    correlation = np.corrcoef(rtt_n, rtt_n_plus_1)[0, 1]
    
    # 创建图表
    plt.figure(figsize=(10, 8))
    
    # 绘制散点图
    plt.scatter(rtt_n, rtt_n_plus_1, alpha=0.6, s=30, color='blue')
    
    # 添加对角线（y=x）
    min_val = min(min(rtt_n), min(rtt_n_plus_1))
    max_val = max(max(rtt_n), max(rtt_n_plus_1))
    plt.plot([min_val, max_val], [min_val, max_val], 'r--', alpha=0.7, label='y=x (Perfect Correlation)')
    
    # 设置坐标轴
    plt.xlabel('RTT of ping #N (milliseconds)')
    plt.ylabel('RTT of ping #N+1 (milliseconds)')
    plt.title(f'RTT Correlation: ping #N vs ping #N+1\nCorrelation Coefficient: {correlation:.3f}')
    
    # 添加网格
    plt.grid(True, alpha=0.3)
    
    # 添加图例
    plt.legend()
    
    # 设置坐标轴范围
    plt.xlim(min_val, max_val)
    plt.ylim(min_val, max_val)
    
    # 自动调整布局
    plt.tight_layout()
    
    # 保存图表
    plt.savefig('rtt_correlation.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    return rtt_n, rtt_n_plus_1, correlation

def analyze_correlation(rtt_n, rtt_n_plus_1, correlation):
    """分析RTT相关性"""
    print("\n=== RTT时间相关性分析 ===")
    print(f"相关系数: {correlation:.4f}")
    
    # 相关系数解释
    if abs(correlation) < 0.1:
        strength = "几乎无相关"
    elif abs(correlation) < 0.3:
        strength = "弱相关"
    elif abs(correlation) < 0.5:
        strength = "中等相关"
    elif abs(correlation) < 0.7:
        strength = "强相关"
    elif abs(correlation) < 0.9:
        strength = "很强相关"
    else:
        strength = "极强相关"
    
    print(f"相关性强度: {strength}")
    
    # 方向性
    if correlation > 0:
        direction = "正相关（第N个RTT高，第N+1个RTT也倾向于高）"
    elif correlation < 0:
        direction = "负相关（第N个RTT高，第N+1个RTT倾向于低）"
    else:
        direction = "无线性相关"
    
    print(f"相关性方向: {direction}")
    
    # 统计检验
    n = len(rtt_n)
    if n > 3:
        # 计算t统计量和p值
        t_stat = correlation * np.sqrt((n-2) / (1 - correlation**2))
        p_value = 2 * (1 - stats.t.cdf(abs(t_stat), n-2))
        
        print(f"\n统计检验:")
        print(f"样本数: {n}")
        print(f"t统计量: {t_stat:.4f}")
        print(f"p值: {p_value:.4f}")
        
        if p_value < 0.001:
            significance = "极显著 (p < 0.001)"
        elif p_value < 0.01:
            significance = "高度显著 (p < 0.01)"
        elif p_value < 0.05:
            significance = "显著 (p < 0.05)"
        else:
            significance = "不显著 (p >= 0.05)"
        
        print(f"统计显著性: {significance}")
    
    # 计算一些统计量
    print(f"\n统计摘要:")
    print(f"第N个RTT - 平均值: {np.mean(rtt_n):.2f} ms, 标准差: {np.std(rtt_n):.2f} ms")
    print(f"第N+1个RTT - 平均值: {np.mean(rtt_n_plus_1):.2f} ms, 标准差: {np.std(rtt_n_plus_1):.2f} ms")
    
    # 计算RTT变化
    rtt_changes = np.array(rtt_n_plus_1) - np.array(rtt_n)
    print(f"RTT变化 - 平均值: {np.mean(rtt_changes):.2f} ms, 标准差: {np.std(rtt_changes):.2f} ms")

def create_lag_analysis(rtts, max_lag=5):
    """创建滞后相关性分析"""
    print(f"\n=== 滞后相关性分析 (lag 1-{max_lag}) ===")
    
    correlations = []
    for lag in range(1, max_lag + 1):
        if len(rtts) > lag:
            rtt_lag = rtts[:-lag]
            rtt_current = rtts[lag:]
            corr = np.corrcoef(rtt_lag, rtt_current)[0, 1]
            correlations.append(corr)
            print(f"Lag {lag}: 相关系数 = {corr:.4f}")
        else:
            correlations.append(np.nan)
            print(f"Lag {lag}: 数据不足")
    
    # 绘制滞后相关性图
    plt.figure(figsize=(8, 6))
    lags = range(1, max_lag + 1)
    valid_corrs = [c for c in correlations if not np.isnan(c)]
    valid_lags = [l for l, c in zip(lags, correlations) if not np.isnan(c)]
    
    if valid_corrs:
        plt.plot(valid_lags, valid_corrs, 'bo-', linewidth=2, markersize=8)
        plt.axhline(y=0, color='r', linestyle='--', alpha=0.7)
        plt.xlabel('Lag')
        plt.ylabel('Correlation Coefficient')
        plt.title('RTT Lag Correlation Analysis')
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        plt.savefig('rtt_lag_correlation.png', dpi=300, bbox_inches='tight')
        plt.show()

def main():
    # 解析数据
    rtts = parse_ping_data('../data/baidu.txt')
    
    if not rtts:
        print("没有找到有效数据")
        return
    
    print(f"总共解析了 {len(rtts)} 个RTT数据点")
    print(f"RTT范围: {min(rtts):.1f} ms 到 {max(rtts):.1f} ms")
    
    # 创建相关性散点图
    result = create_correlation_scatter(rtts)
    if result:
        rtt_n, rtt_n_plus_1, correlation = result
        
        # 分析相关性
        analyze_correlation(rtt_n, rtt_n_plus_1, correlation)
        
        # 滞后相关性分析
        create_lag_analysis(rtts)
        
        # 输出一些示例数据对
        print(f"\n=== 示例相邻RTT对 ===")
        for i in range(min(10, len(rtt_n))):
            print(f"ping #{i+1}: {rtt_n[i]:.1f} ms → ping #{i+2}: {rtt_n_plus_1[i]:.1f} ms")

if __name__ == "__main__":
    main()
# end_aigc
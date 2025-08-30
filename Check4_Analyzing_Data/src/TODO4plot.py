import matplotlib.pyplot as plt
import pandas as pd

# 读取数据
data = pd.read_csv('/root/Stanford_CS144/Check4_Analyzing_Data/build/autocorrelation_data.csv')

# 创建图形
plt.figure(figsize=(12, 6))

# 绘制两组数据
k_values = data['k']
success_probs = data['success_prob']
failure_probs = data['failure_prob']

# 成功→成功的条件概率
plt.bar([x-0.2 for x in k_values], success_probs, width=0.4, 
         label='P(成功|成功)', color='blue', alpha=0.7)

# 丢失→丢失的条件概率  
plt.bar([x+0.2 for x in k_values], failure_probs, width=0.4, 
         label='P(丢失|丢失)', color='red', alpha=0.7)

plt.xlabel('时间偏移 k')
plt.ylabel('条件概率')
plt.title('Ping包丢失的自相关分析')
plt.legend()
plt.grid(True, alpha=0.3)
plt.xticks(k_values)

# 添加参考线
overall_success_rate = success_probs.mean()  # 总体成功率
overall_failure_rate = failure_probs.mean()  # 总体失败率
plt.axhline(y=overall_success_rate, color='blue', linestyle='--', alpha=0.5, 
            label=f'总体成功率: {overall_success_rate:.3f}')
plt.axhline(y=overall_failure_rate, color='red', linestyle='--', alpha=0.5, 
            label=f'总体失败率: {overall_failure_rate:.3f}')

plt.legend()
plt.tight_layout()
plt.savefig('autocorrelation_plot.png', dpi=300, bbox_inches='tight')
plt.show()
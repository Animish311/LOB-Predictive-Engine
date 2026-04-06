import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv('data/lob_dataset.csv', on_bad_lines='skip')
df['datetime'] = pd.to_datetime(df['timestamp'], unit='ms')
df.set_index('datetime', inplace=True)

horizon_ticks = 20 
df['future_mid_price'] = df['mid_price'].shift(-horizon_ticks)
df['fvd'] = df['depth_micro_price'] - df['mid_price']
df['future_return'] = df['future_mid_price'] - df['mid_price']
df.dropna(inplace=True)

#Filtering out the 0.0 flats where order book is balanced 
signal_df = df[df['fvd'] != 0].copy()

#Clipping outlier flash orders for cleaner plot
q_low = signal_df['fvd'].quantile(0.01)
q_high = signal_df['fvd'].quantile(0.99)
signal_df = signal_df[(signal_df['fvd'] > q_low) & (signal_df['fvd'] < q_high)]

signal_df['theoretical_edge'] = np.sign(signal_df['fvd']) * signal_df['future_return']
signal_df['cumulative_edge'] = signal_df['theoretical_edge'].cumsum()


#Plotting
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 7))
fig.patch.set_facecolor('#f4f6f9')

ax1.set_facecolor('#ffffff')
sns.regplot(
    x='fvd', y='future_return', data=signal_df, 
    ax=ax1, 
    scatter_kws={'alpha': 1.0, 'color': '#3498db', 's': 3},
    line_kws={'color': '#e74c3c', 'linewidth': 3}
)

ax1.set_title('Predictive Alpha: Fair Value Deviation vs. Forward Return (+2s)', fontsize=15, fontweight='bold', color='#2c3e50')
ax1.set_xlabel('LOB Weighted Fair Value Deviation', fontsize=12, color='#34495e')
ax1.set_ylabel('Actual Mid-Price Forward Return', fontsize=12, color='#34495e')
ax1.axhline(0, color='#2c3e50', linestyle='--', alpha=0.5)
ax1.axvline(0, color='#2c3e50', linestyle='--', alpha=0.5)
ax1.grid(color='#ecf0f1', linestyle='-', linewidth=1)

ax2.set_facecolor('#ffffff')
ax2.plot(signal_df.index, signal_df['cumulative_edge'], color='#2ecc71', linewidth=2.5)
ax2.fill_between(signal_df.index, signal_df['cumulative_edge'], 0, color='#2ecc71', alpha=0.15)

ax2.set_title('Cumulative Theoretical Edge (Gross P&L)', fontsize=15, fontweight='bold', color='#2c3e50')
ax2.set_xlabel('Time', fontsize=12, color='#34495e')
ax2.set_ylabel('Cumulative Spread Capture (USDT)', fontsize=12, color='#34495e')
ax2.axhline(0, color='#e74c3c', linewidth=1.5, linestyle='--')
ax2.grid(color='#ecf0f1', linestyle='-', linewidth=1)

import matplotlib.dates as mdates
ax2.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
fig.autofmt_xdate

for ax in [ax1, ax2]:
    ax.tick_params(colors='#34495e')
    for spine in ['top', 'right']: ax.spines[spine].set_visible(False)
    for spine in ['left', 'bottom']: ax.spines[spine].set_color('#bdc3c7')

plt.tight_layout()
plt.show()

corr = signal_df['fvd'].corr(signal_df['future_return'])
win_rate = (signal_df['theoretical_edge'] > 0).mean() * 100
print(f"Alpha Correlation: {corr:.4f}")
print(f"Directional Win Rate: {win_rate:.2f}%")

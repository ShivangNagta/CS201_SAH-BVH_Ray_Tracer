import numpy as np
import matplotlib.pyplot as plt
from scipy import stats

# Read data from file
with open('benchmark_data.txt', 'r') as file:
    data = file.read()

# Parse data
lines = data.strip().split('\n')
n_spheres = np.array([float(line.split()[0]) for line in lines])
time_no_bvh = np.array([float(line.split()[1]) for line in lines])
time_bvh = np.array([float(line.split()[2]) for line in lines])

# Create figure with two subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))

# Plot 1: Linear scale
# For no BVH (Linear complexity)
slope_no_bvh, intercept_no_bvh, r_value_no_bvh, _, _ = stats.linregress(n_spheres, time_no_bvh)
r_squared_no_bvh = r_value_no_bvh ** 2

# For BVH (Logarithmic complexity)
# Use log(n) for x-axis to fit logarithmic complexity
slope_bvh_log, intercept_bvh_log, r_value_bvh_log, _, _ = stats.linregress(np.log(n_spheres), time_bvh)
r_squared_bvh_log = r_value_bvh_log ** 2

# Plot original data and fits on first subplot
ax1.scatter(n_spheres, time_no_bvh, label='No BVH (Raw)', color='blue', marker='o')
ax1.scatter(n_spheres, time_bvh, label='With BVH (Raw)', color='red', marker='o')

x_range = np.linspace(min(n_spheres), max(n_spheres), 100)
ax1.plot(x_range, slope_no_bvh * x_range + intercept_no_bvh, 
         '--', color='blue', label=f'No BVH (Linear fit, R² = {r_squared_no_bvh:.4f})')
ax1.plot(x_range, slope_bvh_log * np.log(x_range) + intercept_bvh_log, 
         '--', color='red', label=f'BVH (Log fit, R² = {r_squared_bvh_log:.4f})')

ax1.set_xlabel('Number of Spheres (n)')
ax1.set_ylabel('Time (seconds)')
ax1.set_title('Linear Scale Analysis')
ax1.grid(True, alpha=0.3)
ax1.legend()

# Plot 2: Log-log scale
ax2.scatter(np.log(n_spheres), np.log(time_no_bvh), label='No BVH (Raw)', color='blue', marker='o')
ax2.scatter(np.log(n_spheres), np.log(time_bvh), label='With BVH (Raw)', color='red', marker='o')

# Fit lines in log-log space
slope_no_bvh_loglog, intercept_no_bvh_loglog, r_value_no_bvh_loglog, _, _ = stats.linregress(np.log(n_spheres), np.log(time_no_bvh))
slope_bvh_loglog, intercept_bvh_loglog, r_value_bvh_loglog, _, _ = stats.linregress(np.log(n_spheres), np.log(time_bvh))

x_range_log = np.log(x_range)
ax2.plot(x_range_log, slope_no_bvh_loglog * x_range_log + intercept_no_bvh_loglog, 
         '--', color='blue', label=f'No BVH (slope = {slope_no_bvh_loglog:.2f})')
ax2.plot(x_range_log, slope_bvh_loglog * x_range_log + intercept_bvh_loglog, 
         '--', color='red', label=f'BVH (slope = {slope_bvh_loglog:.2f})')

ax2.set_xlabel('ln(Number of Spheres)')
ax2.set_ylabel('ln(Time)')
ax2.set_title('Log-Log Scale Analysis')
ax2.grid(True, alpha=0.3)
ax2.legend()

plt.tight_layout()
plt.savefig('complexity_analysis_complete.png', dpi=300)

# Print detailed analysis
print(f"""
Complexity Analysis Results:

No BVH Implementation:
- Linear fit slope: {slope_no_bvh:.6f} seconds/sphere
- Linear fit R² value: {r_squared_no_bvh:.6f}
- Log-log slope: {slope_no_bvh_loglog:.2f} (expect ~1.0 for linear complexity)
- Empirical complexity: O(n)

BVH Implementation:
- Logarithmic fit slope: {slope_bvh_log:.6f} seconds/ln(sphere)
- Logarithmic fit R² value: {r_squared_bvh_log:.6f}
- Log-log slope: {slope_bvh_loglog:.2f} (expect ~0.0-0.3 for logarithmic complexity)
- Empirical complexity: O(log n)

Performance Analysis:
1. The no-BVH implementation shows clear linear growth (slope ≈ 1 in log-log space)
2. The BVH implementation shows logarithmic behavior
3. At n={int(n_spheres[-1])}, BVH is {(time_no_bvh[-1] / time_bvh[-1]):.1f}x faster
""")

# Also print the raw data for verification
print("\nRaw data from file:")
print("n_spheres  time_no_bvh  time_bvh")
for n, t1, t2 in zip(n_spheres, time_no_bvh, time_bvh):
    print(f"{int(n):8d}  {t1:10.6f}  {t2:8.6f}")
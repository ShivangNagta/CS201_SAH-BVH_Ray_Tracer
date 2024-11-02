import matplotlib.pyplot as plt
import numpy as np

with open('benchmark_data_bvh_crt.txt', 'r') as file:
    data = file.read()

# Parse data
lines = data.strip().split('\n')
n_spheres = np.array([float(line.split()[0]) for line in lines])
time_bvh = np.array([float(line.split()[1]) for line in lines])


# Plotting
plt.figure(figsize=(10, 5))
plt.plot(n_spheres, time_bvh, marker='o', color='b', linestyle='-')
plt.xlabel("Number of spheres")
plt.ylabel("Time for Intersection Tests (with BVH)")
plt.title("Data Plot")
plt.grid(True)


plt.tight_layout()
plt.savefig('save.png')

plt.show()

from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm
import matplotlib.pyplot as plt
import numpy as np


fig = plt.figure()
ax = fig.add_subplot(121, projection='3d')

x1 = np.linspace(-2, 2, 20)
x2 = np.linspace(-2, 2, 20)
x1, x2 = np.meshgrid(x1, x2)
fx = (x1 ** 2 - 1.5 * x1 * x2 + 2 * x2 ** 2) * x1 ** 2

ax.plot_surface(x1, x2, fx, color='grey')
ax.set_zlim(0, 8)
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

ax2 = fig.add_subplot(122, projection='3d')
cset = ax2.contour(x1, x2, fx, 20, zdir='z', extend3d=False, cmap=cm.coolwarm)
ax2.clabel(cset, fontsize=9, inline=True)
ax2.set_xlabel('X')
ax2.set_ylabel('Y')
ax2.set_zlabel('Z')

plt.show()
# plt.imshow(fx, extent=[0, 5, 0, 5], origin='lower', cmap='RdGy', alpha=0.5)

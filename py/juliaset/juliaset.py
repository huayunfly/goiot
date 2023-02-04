# https://mathworld.wolfram.com/JuliaSet.html
# Draw Julia set in python for understanding, then in CUDA
# @date 2023.01.20

import numpy as np
import matplotlib.pyplot as plt

julia_C = -0.3 + 0.78j


def julia_set(x, y):
   # Make a complex plane.
   z = np.array(x + 1j * y)
   # Iteration
   r = np.zeros(z.shape)
   m = np.ones(z.shape, dtype=bool)
   for i in range(24):
       z[m] = z[m] ** 2 + julia_C
       # Calculate convergence.
       m = np.abs(z) < 2
       # Record the interations, mapping to grey image later.
       # We don't record |z| directly.
       r += m
   return r


def mandelbrot_set(x, y):
   c = np.array(x + 1j * y)
   z = np.zeros(c.shape, dtype=complex)
   r = np.ones(c.shape)
   m = np.ones(c.shape, dtype=bool)
   for i in range(50):
       z[m] = z[m] ** 2 + c[m]
       m = np.abs(z) < 2
       r += m
   return r


def complex_str(c):
   return np.array_str(np.array([julia_C]), suppress_small=True, precision=3)


def grid(width, offset, n):
   x = np.linspace(-width + offset, width + offset, n)
   y = np.linspace(-width, width, n)
   return np.meshgrid(x, y), (x.min(), x.max(), y.min(), y.max())


fig, (ax, bx) = plt.subplots(1, 2)
ax.set_title("Mandelbrot Set(Mirror to Julia Set)")
bx.set_title("Julia Set c=" + complex_str(julia_C))

(X, Y), extent = grid(2, 0, 1000)
cf = ax.imshow(mandelbrot_set(X,Y), extent=extent)

(X, Y), extent = grid(2, 0, 1000)
julia = julia_set(X, Y)
img = bx.imshow(julia, extent=extent, cmap='gray')

def onclick(event):
   if event.inaxes != ax: return
   global X, Y, julia_C
   julia_C = event.xdata + 1j * event.ydata
   julia = julia_set(X, Y)
   img.set_data(julia)
   bx.set_title("Julia Set c=" + complex_str(julia_C))
   fig.canvas.draw_idle()

fig.canvas.mpl_connect('button_press_event', onclick)
plt.tight_layout()
plt.show()

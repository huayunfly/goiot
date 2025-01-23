"""
A playground for ML.
"""

import torch
import uti4ml

# Automatic differentation. The core ideas behind modern backpropagation date to a PhD thesisis Speelpenning, 1980

# A simple case: y = 2 * sum(x**2)
x = torch.arange(4.0, requires_grad=True)
y =  2 * torch.dot(x, x)
print(y)
y.backward()
print(f'1. {x.grad == 4 * x}')


# Another case: y = sum(x)
x.grad.zero_() # Reset the gradient
y = x.sum()
y.backward()
print(f'2. {x.grad == torch.tensor([1., 1., 1., 1.])}')


# Reduces a vector y to the scalar required by pytorch.
x.grad.zero_()
y = x * x
y.backward(gradient=torch.ones(len(y))) # Invoking backward on a non-scalar elicits an error.
# Equals and faster: y.sum().backward()
print(f'3. {x.grad == torch.tensor([0., 2., 4., 6.])}')


# Move some calculations outside of the recorded computational graph.
x.grad.zero_()
y = x * x
u = y.detach() # Detaches to constants
z = u * x
z.sum().backward()
print(f'4. {x.grad == torch.tensor([0., 1., 4., 9.])}')


# Pytorch (tensorflow...) tracks python control flows to record a computational graph.
def f(a):
    b = a * 2
    while b.norm() < 1000:
        b = b * 2
    if b.sum() > 0:
        c = b
    else:
        c = 100 * b
    return c

a = torch.randn(size=(4,), requires_grad=True)
d = f(a)
d.sum().backward()
print(f'5. {a.grad == torch.round(d/a, decimals=4)}')


# Another complex computational graph: log(x**2)*sin(x)+1/x
x0 = torch.randn(())
d_y0 = -x0**(-2) + 2 / x0 * torch.sin(x0) + torch.log(x0**2) * torch.cos(x0)

x = x0.clone().requires_grad_(True)
m1, m3, m5 = 1 / x, torch.sin(x), x ** 2
m4 = torch.log(m5)
m2 = m3 * m4
y = m1 + m2
y.backward()
print(f'6. {torch.round(x.grad, decimals=4) == torch.round(d_y0, decimals=4)}')


# Draw d_sin(x) using autograd
X = uti4ml.np.arange(0.0, 4.0, 0.1)
x = torch.from_numpy(X)
x.requires_grad_(True)
y = torch.sin(x)
y.sum().backward()
Y = [y.detach().numpy(), x.grad.numpy()]
uti4ml.plot(X, Y, 'x', 'f(x)', legend=['sin(x)', 'd_sin(x) by grad'])

# Draw 3D 2024.12.10
X = uti4ml.np.arange(0.0, 4.0, 0.1)
Y = uti4ml.np.arange(0.0, 4.0, 0.1)
X, Y = uti4ml.np.meshgrid(X, Y)
Z = uti4ml.np.sin(uti4ml.np.sqrt(X**2 + Y**2))
uti4ml.plot3D(X, Y, Z, (-1.01, 1.01))
    









 



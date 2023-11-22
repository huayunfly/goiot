'''
NND example 11.8 figure
2023.11.22
'''

import torch

# Parameter definition
w1 = torch.ones((2, 1))
b1 = torch.zeros((2, 1))
w2 = torch.ones((1, 2))
b2 = torch.zeros((1))
w = [torch.tensor([]), w1, w2]
b = [torch.tensor([]), b1, b2]

t_p = torch.arange(-2, 2, 0.2)
t_a = torch.arange((3))
t_t = torch.sin(torch.pi / 4.0 * t_p) + 1
t_v = torch.arange(-2, 2, 0.4)
t_vt = torch.sin(torch.pi / 4.0 * t_v) + 1

# Log sigmoid function
def log_sigmoid_fn(t_n):
    return 1 / (1 + torch.exp(-t_n))

# Linear function
def linear_fn(t_n):
    return t_n

# Empty function
def empty_fn(t_n):
    raise 'Not implemented'

fns = [empty_fn, log_sigmoid_fn, linear_fn]

# Model definition
# t_p: tensor input, t_a: tensor actual output, w1: weights, b1: bias, f1: activation functions
def model(t_p, t_n, t_a, w, b, f):
    t_n[1] = w[1] * t_p[0] + b[1]
    t_a[1] = f[1](t_n[1])
    t_n[2] = w[2] * t_a[1] + b[2]
    t_a[2] = f[2](t_n[2])
    return t_a[2]

# Loss function
def loss_fn(t_t, t_a):
    squared_diffs = (t_t - t_a)**2
    return squared_diffs.mean()

# Gradient function with [weight_grad, bias_grad]
def grad_fn(t_t, t_n, t_a, t_grad, w):
    s2 = torch.diag(torch.ones(t_n[2].shape)) @ ((-2) * (t_t - t_a[2]))
    s1 = torch.diag((1 - t_a[1]) * t_a[1]) @ w[2].t() @ s2 
    t_grad[2] = [s2 @ t_a[1], s2]
    t_grad[1] = [s1 @ t_a[0], s1]


def train(n_epochs, learning_rate, params, f, t_p, t_t):
    t_n = [[],[],[]]
    t_a = [[],[],[]]
    t_grad = [[], [], []]
    for epoch in range(0, n_epochs):
        w, b = params
        pred = model(t_p, t_n, t_a, w, b, f)
        loss = loss_fn(t_t, pred)
        grad_fn(t_t, t_n, t_a, t_grad, w)
        params = params - learning_rate * t_grad
        print('Epoch %d, Loss %f' % (epoch, float(loss)))
    return params
        




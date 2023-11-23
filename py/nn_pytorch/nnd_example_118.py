'''
NND example 11.8 figure
2023.11.22
'''

from matplotlib import pyplot as plt
import torch


# Parameter definition, in batch mode
w1 = torch.ones((2, 1)).unsqueeze(0)
b1 = torch.zeros((2, 1)).unsqueeze(0)
w2 = torch.ones((1, 2)).unsqueeze(0)
b2 = torch.zeros((1, 1)).unsqueeze(0)
w = [torch.empty((0)), w1, w2]
b = [torch.empty((0)), b1, b2]

# train and validate data
train_input = torch.arange(-2, 2, 0.2)
batch_size = train_input.size(dim=0)
t_p = train_input.unsqueeze(1).unsqueeze(2) # (batch_size, 1, 1)
t_n = [torch.empty((0)), torch.zeros((batch_size, 2, 1)), torch.zeros((batch_size, 1, 1))] 
t_a = [t_p, torch.zeros((batch_size, 2, 1)), torch.zeros((batch_size, 1, 1))]
t_t = torch.sin(torch.pi / 4.0 * t_p) + 1
t_grad = [[torch.empty((0)), torch.zeros(w1.shape), torch.zeros(w2.shape)], 
          [torch.empty((0)), torch.zeros(b1.shape), torch.zeros(b2.shape)]]

validate_input = torch.arange(-2, 2, 0.4)
t_v = validate_input.unsqueeze(1).unsqueeze(2)
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

# Model definition in batch
# t_p: tensor input, t_a: tensor actual output, w1: weights, b1: bias, f1: activation functions
def model(t_p, t_n, t_a, w, b, fn):
    t_n[1] = w[1] @ t_p + b[1]
    t_a[1] = fn[1](t_n[1])
    t_n[2] = w[2] @ t_a[1] + b[2]
    t_a[2] = fn[2](t_n[2])
    return t_a[2]

# Loss function in batch
def loss_fn(t_t, t_a):
    squared_diffs = (t_t - t_a)**2
    return squared_diffs.mean()

# Gradient function with [weight_grad, bias_grad]
def grad_fn(t_t, t_n, t_a, t_grad, w):
    s2 = torch.ones(t_n[2].shape) * ((-2) * (t_t - t_a[2]))
    s1 = (1 - t_a[1]) * t_a[1] * torch.transpose(w[2], 1, 2) @ s2 
    t_grad[0][2] = s2 @ torch.transpose(t_a[1], 1, 2)
    t_grad[0][1] = s1 @ torch.transpose(t_a[0], 1, 2)
    t_grad[1][2] = s2
    t_grad[1][1] = s1

# Train in batch mode
def train(n_epochs, learning_rate, params, fn, p, t, n, a, grad):
    for epoch in range(0, n_epochs):
        w, b = params
        pred = model(p, n, a, w, b, fn)
        loss = loss_fn(t_t, pred)
        grad_fn(t, n, a, grad, w)
        for num in range(1, len(params[0])):
            params[0][num] -= torch.mean(learning_rate * grad[0][num], 0).unsqueeze(0)
            params[1][num] -= torch.mean(learning_rate * grad[1][num], 0).unsqueeze(0)
        if epoch % 100 == 0:
            print('Epoch %d, Loss %f' % (epoch, float(loss)))
    return params

params = train(1000, 1e-2, [w, b], fns, t_p, t_t, t_n, t_a, t_grad)

# Draw
fig = plt.figure(dpi=600)
plt.xlabel("input")
plt.ylabel("output")
plt.plot(t_p.squeeze(2).numpy(), t_t.squeeze(2).numpy(), 'o')
plt.show()

        




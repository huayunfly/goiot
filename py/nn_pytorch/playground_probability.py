"""
A playground for Probabilities.
"""


# %matplotlib inline
import random
import torch
from torch.distributions.multinomial import Multinomial
import uti4ml

num_tosses = 100
heads = sum([random.random() > 0.5 for _ in range(num_tosses)])
tails = num_tosses - heads
print(f'Toss coin by number {num_tosses} where as heads, tails: ', [heads, tails])

# Frequency of outcome for the repeated tossing coin events.
fair_probs = torch.tensor([0.5, 0.5])
counts = Multinomial(100, fair_probs).sample()
print('Multinomial probability: ', counts / 100)

# Law of large number and central limit theorem.
counts = Multinomial(1, fair_probs).sample((10000,))
cum_counts = counts.cumsum(dim=0)
estimates = cum_counts / cum_counts.sum(dim=1, keepdim=True)
estimates = estimates.numpy()
uti4ml.plot(torch.arange(10000).numpy(), [estimates[:, 0], estimates[:, 1]],
            'Samples', 'Estimated probability', ['P(coin=heads)', 'P(coins=tails)'])













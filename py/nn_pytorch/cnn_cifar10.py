"""
Convolution NN using cifar10 dataset
@date 2023-12-18
"""

import torch
from torch import nn
from torchvision import datasets
from torchvision import transforms
import torch.nn.functional as F
import datetime
from matplotlib import pyplot as plt


data_path = 'data/CIFAR10'
do_preprocess = False

if do_preprocess:
# Download, preprocess data for normalization
    cifar10 = datasets.CIFAR10(data_path, train=True, download=False)
    cifar10_val = datasets.CIFAR10(data_path, train=False, download=False)

    img, label = cifar10[177]
    plt.imshow(img)
    plt.show()

    tensor_cifar10 = datasets.CIFAR10(data_path, train=True, download=False, 
                                  transform=transforms.ToTensor())
    imgs = torch.stack([img for img, _ in tensor_cifar10], dim=3)
    mean = imgs.view(3, -1).mean(dim=1)
    std = imgs.view(3, -1).std(dim=1)

    tensor_cifar10_val = datasets.CIFAR10(data_path, train=False, download=False,
                                      transform=transforms.ToTensor())
    imgs_val = torch.stack([img for img, _ in tensor_cifar10_val], dim=3)
    mean_val = imgs_val.view(3, -1).mean(dim=1)
    std_val = imgs_val.view(3, -1).std(dim=1)

# Normalization
transformed_cifar10 = datasets.CIFAR10(data_path, train=True, download=False, 
                                       transform=transforms.Compose([
                                           transforms.ToTensor(),
                                           transforms.Normalize(mean=[0.4914, 0.4822, 0.4465], std=[0.2470, 0.2435, 0.2616])
                                       ]))

transformed_cifar10_val = datasets.CIFAR10(data_path, train=False, download=False,
                                           transform=transforms.Compose([
                                               transforms.ToTensor(),
                                               transforms.Normalize(mean=[0.4942, 0.4851, 0.4504], std=[0.2467, 0.2429, 0.2616])
                                           ]))


# My model
class CNet(nn.Module):
    def __init__(self, n_channels_1: int):
        super().__init__()
        self.n_channels_1 = n_channels_1
        self.conv1 = nn.Conv2d(3, n_channels_1, kernel_size=3, padding=1)
        self.conv2 = nn.Conv2d(n_channels_1, n_channels_1 // 2, kernel_size=3, padding=1)
        self.fc1 = nn.Linear(8 * 8 * n_channels_1 // 2, 32)
        self.fc2 = nn.Linear(32, 10)
        
    def forward(self, x):
        out = F.max_pool2d(torch.tanh(self.conv1(x)), kernel_size=2)
        out = F.max_pool2d(torch.tanh(self.conv2(out)), kernel_size=2)
        out = out.view(-1, 8 * 8 * self.n_channels_1 // 2)
        out = torch.tanh(self.fc1(out))
        out = self.fc2(out)
        return out


class CNetDropout(nn.Module):
    '''
    Dropout: a Simple Way to Prevent Neural Networks from Overfitting
    '''
    def __init__(self, n_channels_1: int):
        super().__init__()
        self.n_channels_1 = n_channels_1
        self.conv1 = nn.Conv2d(3, n_channels_1, kernel_size=3, padding=1)
        self.conv1_dropout = nn.Dropout(p = 0.4)
        self.conv2 = nn.Conv2d(n_channels_1, n_channels_1 // 2, kernel_size=3, padding=1)
        self.conv2_dropout = nn.Dropout(p = 0.4)
        self.fc1 = nn.Linear(8 * 8 * n_channels_1 // 2, 32)
        self.fc2 = nn.Linear(32, 10)
        
    def forward(self, x):
        out = F.max_pool2d(torch.tanh(self.conv1(x)), kernel_size=2)
        out = self.conv1_dropout(out)
        out = F.max_pool2d(torch.tanh(self.conv2(out)), kernel_size=2)
        out = self.conv2_dropout(out)
        out = out.view(-1, 8 * 8 * self.n_channels_1 // 2)
        out = torch.tanh(self.fc1(out))
        out = self.fc2(out)
        return out


class CNetBatchNorm(nn.Module):
    '''
    Batch normalization allowing us to increase the learning rate and make training less dependent 
    on initialization and act as a regularizer, thus representing an alternative to dropout.
    '''
    def __init__(self, n_channels_1: int):
        super().__init__()
        self.n_channels_1 = n_channels_1
        self.conv1 = nn.Conv2d(3, n_channels_1, kernel_size=3, padding=1)
        self.conv1_batchnorm = nn.BatchNorm2d(num_features=n_channels_1)
        self.conv2 = nn.Conv2d(n_channels_1, n_channels_1 // 2, kernel_size=3, padding=1)
        self.conv2_batchnorm = nn.BatchNorm2d(num_features=n_channels_1 // 2)
        self.fc1 = nn.Linear(8 * 8 * n_channels_1 // 2, 32)
        self.fc2 = nn.Linear(32, 10)
        
    def forward(self, x):
        out = self.conv1_batchnorm(self.conv1(x))
        out = F.max_pool2d(torch.tanh(out), kernel_size=2)
        out = self.conv2_batchnorm(self.conv2(out))
        out = F.max_pool2d(torch.tanh(out), kernel_size=2)
        out = out.view(-1, 8 * 8 * self.n_channels_1 // 2)
        out = torch.tanh(self.fc1(out))
        out = self.fc2(out)
        return out
    

class ResBlock(nn.Module):
    def __init__(self, n_channels):
        super(ResBlock, self).__init__()
        self.conv = nn.Conv2d(n_channels, n_channels, kernel_size=3, padding=1, bias=False)
        self.batch_norm = nn.BatchNorm2d(num_features=n_channels)
        nn.init.kaiming_normal_(self.conv.weight, nonlinearity='relu')
        nn.init.constant_(self.batch_norm.weight, 0.5)
        nn.init.zeros_(self.batch_norm.bias)

    def forward(self, x):
        out = self.conv(x)
        out = self.batch_norm(out)
        out = torch.relu(out)
        return out + x


class CNetRes(nn.Module):
    '''
    Residual network.
    32 channels, 50 blcoks, 15min - 10epochs on i9-13900k
    '''
    def __init__(self, n_channels_1=32, n_blocks=10):
        super().__init__()
        self.n_channels = n_channels_1
        self.conv1 = nn.Conv2d(3, n_channels_1, kernel_size=3, padding=1)
        self.res_blocks = nn.Sequential(*(n_blocks * [ResBlock(n_channels=n_channels_1)]))
        self.fc1 = nn.Linear(8 * 8 * n_channels_1, 32)
        self.fc2 = nn.Linear(32, 10)

    def forward(self, x):
        out = F.max_pool2d(torch.relu(self.conv1(x)), kernel_size=2)
        out = self.res_blocks(out)
        out = F.max_pool2d(torch.relu(out), kernel_size=2)
        out = out.view(-1, 8 * 8 * self.n_channels)
        out = torch.relu(self.fc1(out))
        return self.fc2(out)


device = (torch.device('cuda') if torch.cuda.is_available()
else torch.device('cpu'))
print(f"Training on device {device}.")
# Train loop
def training_loop(n_epochs, optimizer, model, loss_fn, train_loader):
    for epoch in range(1, n_epochs + 1):
        loss_train = 0.0
        for imgs, labels in train_loader:
            imgs = imgs.to(device=device)
            labels = labels.to(device=device)
            outputs = model(imgs)
            loss = loss_fn(outputs, labels)
            optimizer.zero_grad(set_to_none=True)
            loss.backward()
            optimizer.step()
            loss_train += loss.item()
        if epoch == 1 or epoch % 10 == 0:
            print('{} Epoch {}, Training loss {}'.format(
                datetime.datetime.now(), epoch, loss_train / len(train_loader)))


def validate(model, train_loader, val_loader):
    for name, loader in [('train', train_loader), ('validate', val_loader)]:
        correct = 0
        count = 0
        with torch.no_grad():
            for imgs, labels in loader:
                imgs = imgs.to(device=device)
                labels = labels.to(device=device)
                outputs = model(imgs)
                _, indices = torch.max(outputs, dim=1)
                correct += int((labels == indices).sum())
                count += labels.shape[0]
            print('Accuracy {}: {:.2f}'.format(name, correct / count))


# Train and validate
model = CNetRes(n_channels_1=32, n_blocks=50).to(device=device)
print('Total parameters {}'.format(sum(p.numel() for p in model.parameters())))
#img_ts, _ = transformed_cifar10[177]
#model(img_ts.unsqueeze(0))          
train_loader = torch.utils.data.DataLoader(transformed_cifar10, batch_size=64, shuffle=True)
val_loader = torch.utils.data.DataLoader(transformed_cifar10_val, batch_size=64, shuffle=False)
optimizer = torch.optim.SGD(model.parameters(), lr=1e-2)
loss_fn = nn.CrossEntropyLoss()

training_loop(n_epochs = 100,
              optimizer = optimizer,
              model=model,
              loss_fn = loss_fn, 
              train_loader = train_loader)
torch.save(model.state_dict(), data_path + 'ten_classes.pt')
validate(model, train_loader, val_loader)









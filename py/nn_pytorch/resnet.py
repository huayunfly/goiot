import torch
from torchvision import models
from torchvision import transforms
from PIL import Image
import time
import ssl

from cyclegan import ResNetGenerator

# Disable 'unable to get local issuer certificate' warning while downloading nn weights.
ssl._create_default_https_context = ssl._create_unverified_context

# Get cpu or gpu device for training.
my_device = "cuda" if torch.cuda.is_available(
) else "mps" if torch.backends.mps.is_available() else "cpu"
print(f"Using {my_device} device")

resnet = models.resnet101(weights=models.ResNet101_Weights.DEFAULT).to(device=my_device)
#print(resnet)
preprocess = transforms.Compose([
    transforms.Resize(256), 
    transforms.CenterCrop(224),
    transforms.ToTensor(),
    transforms.Normalize(
        mean=[0.485, 0.456, 0.406], 
        std=[0.229, 0.224, 0.225]
        )])
img = Image.open("./data/resnet/cat_1.jpg")
img_t = preprocess(img).to(device=my_device)

# Model inference
time_begin = time.time()
batch_t = torch.unsqueeze(img_t, 0)
resnet.eval()
out = resnet(batch_t)
time_end = time.time()
print(f'time elpased -> {time_end - time_begin}s')

# Match output to label based on imageNet_classes & synsets.
with open('./data/resnet/imagenet_classes.txt') as f:
    labels = [line.strip() for line in f.readlines()]
with open('./data/resnet/imagenet_synsets.txt') as f:
    label_notes : {str : str} = {line.split(' ', 1)[0]: line.split(' ', 1)[1] for line in f.readlines()}
_, index = torch.max(out, 1)
percentage = torch.nn.functional.softmax(out, dim=1)[0] * 100
#print('Target: %s, with probability %f' % (label_notes[labels[index[0]]].rstrip(), percentage[index[0]].item()))

# Top 5 sorted on output
_, indices = torch.sort(out, descending=True)
for idx in indices[0][:5]:
    print('Target: %s, with probability %f' % (label_notes[labels[idx]].rstrip(), percentage[idx].item()))

# Let's turn to GAN
netG = ResNetGenerator()
model_path = './data/resnet/horse2zebra_0.4.0.pth'
model_data = torch.load(model_path)
netG.load_state_dict(model_data)
netG.eval()
#print(netG)
preprocess = transforms.Compose([transforms.Resize(256), transforms.ToTensor()])
img = Image.open('./data/resnet/horse_1.jpg')
img_t = preprocess(img)
batch_t = torch.unsqueeze(img_t, 0)
batch_out = netG(batch_t)
out_t = (batch_out.data.squeeze() + 1.0) / 2.0
out_img = transforms.ToPILImage()(out_t)
out_img.show()




import cv2
import numpy as np
import math
import time
from matplotlib import pyplot as plt
from cv2 import threshold, drawContours

def roi_mask(img, vertices):
    mask = np.zeros_like(img)
    if len(img.shape) > 2:
        channel_count = img.shape[2]
        mask_color = (255,) * channel_count
    else:
        mask_color = 255
    cv2.fillPoly(mask, vertices, mask_color)
    masked_img = cv2.bitwise_and(img, mask)
    return masked_img

img = cv2.imread('sample1.jpg')
kernel = np.ones((5, 5), np.uint8)
start_time = time.time()
# convert the image to grayscale
imgray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

#canny 
# 1. 使用高斯模糊，去除噪音点（cv2.GaussianBlur）    
# 2. 灰度转换（cv2.cvtColor）    
# 3. 使用sobel算子，计算出每个点的梯度大小和梯度方向    
# 4. 使用非极大值抑制(只有最大的保留)，消除边缘检测带来的杂散效应    
# 5. 应用双阈值，来确定真实和潜在的边缘    
# 6. 通过抑制弱边缘来完成最终的边缘检测
edges = cv2.Canny(imgray, 60, 360, apertureSize=3, L2gradient=True)

#roi_vtx = np.array([[(0, img.shape[0]), (100, 100), (300, 300), (img.shape[1], img.shape[0])]])
side_len = 80
start_x = 110
start_y = 220
roi_vtx = np.array([[(start_x, start_y), (start_x + side_len, start_y), (start_x + side_len, start_y + side_len), (start_x, start_y + side_len)]])
roi_edges = roi_mask(edges, roi_vtx)
# For each and every pixel having value 1 in box 1, find a pixel having a value 1 in box 2.
value1 = np.where(roi_edges == 255)
average_distance = np.average(start_y - value1[1])

end_time = time.time()
time_cost = end_time - start_time
print('time cost %0.5f sec' %time_cost)

# Plot
plt.subplot(131), plt.imshow(img, cmap='gray')
plt.title('Original Image'), plt.xticks([]), plt.yticks([])
plt.subplot(132), plt.imshow(edges, cmap='gray')
plt.title('Canny'), plt.xticks([]), plt.yticks([])
plt.subplot(133), plt.imshow(roi_edges, cmap='gray')
plt.title('ROI'), plt.xticks([]), plt.yticks([])
plt.show()

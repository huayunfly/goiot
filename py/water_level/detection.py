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

img = cv2.imread('sample3.jpg')
kernel = np.ones((5, 5), np.uint8)
start_time = time.time()
# convert the image to grayscale
imgray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
imgray_contours = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

#roi_vtx = np.array([[(0, img.shape[0]), (100, 100), (300, 300), (img.shape[1], img.shape[0])]])
side_len = 250
start_x = 100
start_y = 200
roi_vtx = np.array([[(start_x, start_y), (start_x + side_len, start_y), (start_x + side_len, start_y + side_len), (start_x, start_y + side_len)]])
roi_mask_edges = roi_mask(imgray, roi_vtx)
roi_edges = imgray[start_y: start_y + side_len, start_x: start_x + side_len]
# For each and every pixel having value 1 in box 1, find a pixel having a value 1 in box 2.
value1 = np.where(roi_edges == 255)
average_distance = np.average(start_y - value1[1])

# Canny edge detector [Canny86]
# 1. 使用高斯模糊，去除噪音点（cv2.GaussianBlur）    
# 2. 灰度转换（cv2.cvtColor）    
# 3. 使用sobel算子(设置apertureSize)，计算出每个点的x,y向梯度大小，组合成4个方向导数    
# 4. 保留局部方向导数最大值点（非极大值抑制），消除边缘检测带来的杂散效应    
# 5. 应用双阈值，推荐2：1到3：1，梯度>大值，视作边缘候选点。梯度<小值，丢弃。小值<梯度<大值，与大值边缘点连接的，保留。    
# 6. 将边缘候选点组装为轮廓。
# 7. 最后一个参数指示计算方向梯度算法，true: L2-norm = sqrt((dI/dx)^2 + (dI/dy)^2)
#    false: L1-norm = abs(dI/dx) + abs(dI/dy)
lower_threshold = 15
upper_threshold = 30
edges = cv2.Canny(roi_edges, lower_threshold, upper_threshold, apertureSize=3, L2gradient=True)

# Find contours
contours_and_hierarchy = cv2.findContours(edges, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
# Filter nearly horizontal lines
horizon_line_idx = []
direction = np.tan(np.pi * (15 / 180))
min_line_len = 30
# Section
section_num = 4
lines_in_section = [0] * section_num
for i in range(0, len(contours_and_hierarchy[0])):
    if cv2.arcLength(contours_and_hierarchy[0][i], True) < min_line_len:
        continue
    line = cv2.fitLine(contours_and_hierarchy[0][i], cv2.DIST_L2, 0, 0.01, 0.01)
    # line structure: (vx, vy, x0, y0)
    if abs(line[1] / line[0]) < direction:
        horizon_line_idx.append(i)
        lines_in_section[int(line[3] / (side_len / section_num))] += 1
for i in horizon_line_idx:
    # Add ROI x-y offset
    for k in range(0, len(contours_and_hierarchy[0][i])):
        contours_and_hierarchy[0][i][k][0][0] += start_x
        contours_and_hierarchy[0][i][k][0][1] += start_y
    cv2.drawContours(imgray_contours, contours_and_hierarchy[0], i, (0, 0, 255), 2)

MIN_LINES = 5
MIN_RATIO = 0.7
if len(horizon_line_idx) > MIN_LINES:
    # Get the max fitted lines ratio in ROI sections
    ratio_in_section = list(map(lambda i : i / len(horizon_line_idx), lines_in_section))
    max_ratio = max(ratio_in_section)
    max_index = ratio_in_section.index(max_ratio)
    print('max fitted line at index[%d] with ratio %.1f' %(max_index, max_ratio))
    
    if max_ratio > MIN_RATIO:
        # Draw liquid level (the middle line of the section)
        level_y = int(start_y + max_index * side_len / section_num + side_len / section_num / 2)
        cv2.line(imgray_contours, (start_x, level_y), (start_x + side_len, level_y), (0, 255, 0), 2)

# Last: draw ROI border
cv2.drawContours(imgray_contours, [roi_vtx], 0, (255, 0, 0), 2)

end_time = time.time()
time_cost = end_time - start_time
print('time cost %0.3f sec' %time_cost)

# Plot
plt.subplot(141), plt.imshow(img, cmap='gray')
plt.title('Original Image'), plt.xticks([]), plt.yticks([])
plt.subplot(142), plt.imshow(roi_mask_edges, cmap='gray')
plt.title('ROI'), plt.xticks([]), plt.yticks([])
plt.subplot(143), plt.imshow(edges, cmap='gray')
plt.title('Canny'), plt.xticks([]), plt.yticks([])
plt.subplot(144), plt.imshow(imgray_contours, cmap='gray')
plt.title('Contours'), plt.xticks([]), plt.yticks([])
plt.show()

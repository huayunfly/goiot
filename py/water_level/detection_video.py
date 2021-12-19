import cv2
import numpy as np
import math
import time
from matplotlib import pyplot as plt
from cv2 import DIST_L2, threshold, drawContours
from numpy.lib.function_base import average


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


# Interactive plot
plt.ion()
# fig = plt.figure()
# original_plot = fig.add_subplot(131)
# canny_plot = fig.add_subplot(132)
# roi_plot = fig.add_subplot(133)

cap = cv2.VideoCapture(0)
if cap.isOpened == False:
    print("Error opening video stream")

while (cap.isOpened()):
    ret, frame = cap.read()
    if ret == True:

        start_time = time.time()
        # convert the image to grayscale
        imgray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # canny
        # 1. 使用高斯模糊，去除噪音点（cv2.GaussianBlur）
        # 2. 灰度转换（cv2.cvtColor）
        # 3. 使用sobel算子，计算出每个点的梯度大小和梯度方向
        # 4. 使用非极大值抑制(只有最大的保留)，消除边缘检测带来的杂散效应
        # 5. 应用双阈值，来确定真实和潜在的边缘
        # 6. 通过抑制弱边缘来完成最终的边缘检测
        edges = cv2.Canny(imgray, 60, 360, apertureSize=3, L2gradient=True) 

        # threshold
        ret, binary = cv2.threshold(imgray, 127, 255, cv2.THRESH_BINARY)

        # find contours
        contours, hierarchy = cv2.findContours(edges, cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE) 
        # sort max contour length
        sort_idx = [i for i in range(len(contours))]
        lengths = list(map(lambda i: cv2.arcLength(contours[i], False), sort_idx))
        #sort_idx.sort(key=lambda idx : lengths[idx], reverse=True)
        # filter
        lines_idx = list(filter(lambda i : lengths[i] > 80 and lengths[i] < 300, sort_idx))

        # old match_idx
        # match_idx = list(filter(lambda i : max(
        #     contours[i][:, 0][:, 1]) < 240 and min(contours[i][:, 0][:, 1]) > 80, lines_idx))

        # match_idx using fitLine
        lines = [cv2.fitLine(contours[i], DIST_L2, 0, 1e-2, 1e-2) for i in lines_idx]
        match_lines = list(filter(lambda i : lines[i][3] < 240 and lines[i][3] > 120, range(len(lines_idx))))
        match_idx = [lines_idx[i] for i in match_lines]
        # match_idx = list(filter(lambda i : max(
        #     contours[i][:, 0][:, 1]) - min(contours[i][:, 0][:, 1]) < 120, match_idx))  

        for i in match_lines:
            cv2.drawContours(frame, contours, lines_idx[i], (0, 0, 255), 3)

        side_len = 120
        start_x = 180
        start_y = 110
        roi_vtx = np.array([[(start_x, start_y), (start_x + side_len, start_y),
                        (start_x + side_len, start_y + side_len), (start_x, start_y + side_len)]])
        roi_edges = roi_mask(edges, roi_vtx)
        # For each and every pixel having value 1 in box 1, find a pixel having a value 1 in box 2.
        value1 = np.where(roi_edges == 255)
        average_distance = np.average(start_y - value1[1])

        end_time = time.time()
        time_cost = end_time - start_time
        print('time cost %0.5f sec' % time_cost)

        plt.subplot(141), plt.imshow(frame, cmap='gray')
        plt.title('Original Image'), plt.xticks([]), plt.yticks([])
        plt.subplot(142), plt.imshow(edges, cmap='gray')
        plt.title('Canny'), plt.xticks([]), plt.yticks([])
        plt.subplot(143), plt.imshow(roi_edges, cmap='gray')
        plt.title('ROI'), plt.xticks([]), plt.yticks([])
        plt.subplot(144), plt.imshow(binary, cmap='gray')
        plt.title('threshold'), plt.xticks([]), plt.yticks([])
        plt.pause(0.1)
        plt.clf()

        # Press Q on keyboard to  exit
        if cv2.waitKey(25) & 0xFF == ord('q'):
            break
    else:
        break

# When everything done, release the video capture object
cap.release()
# Closes all the frames
cv2.destroyAllWindows()

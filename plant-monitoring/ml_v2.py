from pathlib import Path
import pathlib
import torch
import cv2
import os
from tensorflow.keras.models import load_model
import numpy as np
import tensorflow as tf


temp = pathlib.WindowsPath
pathlib.WindowsPath = pathlib.PosixPath

# Load model YOLOv5
model = torch.hub.load('ultralytics/yolov5', 'custom', path='D:/altha/Kuliah/Semester 8/if4051_smart-farming/plant-monitoring/lettuce_segmentation_model.pt')
model.conf = 0.4

# Input & Output Folder
input_folder = r'D:\altha\Kuliah\Semester 8\if4051_smart-farming\plant-monitoring\input'
output_folder = 'output'
os.makedirs(output_folder, exist_ok=True)

image_extensions = ['.jpg']
image_files = [f for f in os.listdir(input_folder) if os.path.splitext(f)[1].lower() in image_extensions]

for img_file in image_files:
    img_full_path = os.path.join(input_folder, img_file)
    img = cv2.imread(img_full_path)

    if img is None:
        print(f"Gagal membaca {img_file}")
        continue

    img = cv2.resize(img, (640, 480))
    results = model(img)
    detections = results.xyxy[0]
    class_names = model.names
    base_filename = os.path.splitext(img_file)[0]

    label_positions = []

    for i, det in enumerate(detections):
        x1, y1, x2, y2 = map(int, det[:4])
        class_id = int(det[5])
        isReady = "R" if class_names[class_id] == "Lettuce-Ready-to-Harvest" else "NR"

        color = (0, 0, 255)
        label_text = f'{isReady}'

        font_scale = 0.9
        thickness = 2

        (text_w, text_h), _ = cv2.getTextSize(label_text, cv2.FONT_HERSHEY_SIMPLEX, font_scale, thickness)
        text_x = x1
        text_y = y1 - 10 if y1 - text_h - 10 > 0 else y1 + text_h + 10

        while any(abs(text_y - prev_y) < text_h + 5 and abs(text_x - prev_x) < text_w + 5 for prev_x, prev_y in label_positions):
            text_y += text_h + 5

        label_positions.append((text_x, text_y))

        cv2.rectangle(img, (x1, y1), (x2, y2), color, 2)
        cv2.putText(img, label_text, (text_x, text_y - 5), cv2.FONT_HERSHEY_SIMPLEX, font_scale, color, thickness)

    output_img_path = os.path.join(output_folder, f'{base_filename}_boxed.jpg')
    cv2.imwrite(output_img_path, img)
    print(f"Disimpan: {output_img_path}")

print("END")
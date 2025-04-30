import cv2
from datetime import datetime
import os

def capture_image():
    cam = cv2.VideoCapture(0) 
    ret, frame = cam.read()
    
    if not ret:
        print("Failed to capture image")
        return None

    base_path = "/home/dhanikalisa/projects/python/if4051_smart-farming/plant-monitoring"
    image_dir = os.path.join(base_path, "images")
    os.makedirs('images', exist_ok=True)
    filename = datetime.now().strftime("%Y%m%d_%H%M%S.jpg")
    filepath = os.path.join(image_dir, filename)
    cv2.imwrite(filepath, frame)
    cam.release()
    print(f"Image saved as {filepath}")
    return filepath

if __name__ == "__main__":
    capture_image()
    
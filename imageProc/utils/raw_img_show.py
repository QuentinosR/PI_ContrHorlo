import numpy as np
import cv2

# Example usage:
file_path = "image_18.raw"  # Path to your image file

# Define the dimensions of the image
width = 1440  # Width of the image
height = 1080  # Height of the image

# Read the raw data into a numpy array
with open(file_path, 'rb') as f:
    raw_data = np.fromfile(f, dtype=np.uint8)

# Reshape the array to match the dimensions of the image
image_data = raw_data.reshape((height, width))

# Convert Bayer pattern image to RGB
rgb_image = cv2.cvtColor(image_data, cv2.COLOR_BayerBG2RGB)  # Adjust Bayer pattern type as per your raw image

# Save the RGB image
cv2.imwrite('output.png', rgb_image)

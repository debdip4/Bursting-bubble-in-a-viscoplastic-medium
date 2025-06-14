#!/bin/bash

echo "--- Compiling post-processing tools ---"
# Compile the interface getter (can be reused)
qcc -Wall -O2 get_bubble_interface.c -o get_bubble_interface -lm
# Compile the new field getter
qcc -Wall -O2 get_bubble_fields.c -o get_bubble_fields -lm

echo "--- Generating colorful video frames ---"
python3 make_colorful_video.py

echo "--- Encoding final video ---"
ffmpeg -framerate 25 -i bubble_colorful_frames/frame_%04d.png -c:v libx264 -pix_fmt yuv420p -y bubble_bursting_colorful_video.mp4

echo "--- Done! Video saved as bubble_bursting_colorful_video.mp4 ---"
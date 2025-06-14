# file: make_bubble_video.py
import numpy as np
import os
import subprocess as sp
import matplotlib.pyplot as plt

# --- Configuration ---
n_snapshots = 250  # Number of frames to generate (adjust as needed based on your tmax and tsnap)
tsnap = 0.02       # The time interval for PPM/snapshot saving in burstingBubble_fast.c
output_folder = 'bubble_video_frames'
# --------------------

if not os.path.isdir(output_folder):
    os.makedirs(output_folder)

# Function to get interface data by running the C code
def get_interface(snapshot_file):
    exe = ["./get_bubble_interface", snapshot_file]
    p = sp.Popen(exe, stdout=sp.PIPE, stderr=sp.PIPE)
    stdout, stderr = p.communicate()
    
    if p.returncode != 0:
        print(f"Error processing {snapshot_file}:")
        print(stderr.decode())
        return None
        
    lines = stdout.decode().strip().split('\n')
    segs = []
    for i in range(0, len(lines), 2):
        if i + 1 < len(lines):
            try:
                p1 = list(map(float, lines[i].split()))
                p2 = list(map(float, lines[i+1].split()))
                # Basilisk axi: x is vertical, y is radial
                # We plot x horizontally and y vertically
                segs.append(((p1[1], p1[0]), (p2[1], p2[0]))) # (r, z) format
                # Add the reflection for the other half of the bubble
                segs.append(((-p1[1], p1[0]), (-p2[1], p2[0])))
            except (ValueError, IndexError):
                continue
    return segs

# Main loop to generate frames
for i in range(n_snapshots):
    # This assumes your snapshots are named based on the integer 'i'
    # which is the best practice I recommended earlier.
    # If they are still named by time 't', you'll need to adjust this line.
    snapshot_filename = f"intermediate/snapshot-{i*tsnap:5.4f}" # Adjust if naming scheme is different
    output_filename = f"{output_folder}/frame_{i:04d}.png"
    
    if not os.path.exists(snapshot_filename):
        print(f"Snapshot {snapshot_filename} not found, stopping.")
        break
        
    print(f"Processing {snapshot_filename} -> {output_filename}")
    
    segments = get_interface(snapshot_filename)
    
    if segments:
        fig, ax = plt.subplots(figsize=(10, 8))
        
        for seg in segments:
            ax.plot([seg[0][0], seg[1][0]], [seg[0][1], seg[1][1]], 'b-') # Blue line for interface
            
        # --- Plotting settings ---
        ax.set_aspect('equal')
        ax.set_xlim(-2.0, 2.0)
        ax.set_ylim(-1.5, 2.5)
        ax.set_xlabel("r")
        ax.set_ylabel("z")
        ax.set_title(f"Bubble Bursting, Time = {i*tsnap:.2f}")
        ax.grid(True, linestyle='--', alpha=0.6)
        
        plt.savefig(output_filename, bbox_inches='tight')
        plt.close(fig)

print("Frame generation complete.")
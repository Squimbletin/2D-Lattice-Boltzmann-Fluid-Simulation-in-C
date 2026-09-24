"""Turn the PPM frames written by wind_tunnel into an animated GIF.

Usage:  python tools/make_gif.py [frames_dir] [output.gif] [fps] [downscale]
Needs:  pip install pillow
"""
import glob
import sys

from PIL import Image

frames_dir = sys.argv[1] if len(sys.argv) > 1 else "frames"
out_path = sys.argv[2] if len(sys.argv) > 2 else "wind_tunnel.gif"
fps = float(sys.argv[3]) if len(sys.argv) > 3 else 15
downscale = float(sys.argv[4]) if len(sys.argv) > 4 else 1.0

paths = sorted(glob.glob(f"{frames_dir}/frame_*.ppm"))
if not paths:
    sys.exit(f"No frames found in {frames_dir}/ - run ./wind_tunnel first")

frames = []
for p in paths:
    img = Image.open(p).convert("RGB")
    if downscale != 1.0:
        img = img.resize((int(img.width / downscale), int(img.height / downscale)), Image.LANCZOS)
    frames.append(img)

frames[0].save(out_path, save_all=True, append_images=frames[1:],
               duration=int(1000 / fps), loop=0, optimize=True)
print(f"Wrote {out_path} ({len(frames)} frames)")

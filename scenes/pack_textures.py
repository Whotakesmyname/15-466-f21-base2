import numpy as np
from PIL import Image
import struct


# Earth textures will be packed as
# daytime texture
# (uint32) width height (uint8) data*(width*height*3)
# night texture
# (uint32) width height (uint8) data*(width*height*3)
# cloud texture
# (uint32) width height (uint8) data*(width*height)


f = open("../dist/NTFC.tex", "wb")

print("3 textures left...")

im = Image.open("earth_daytime.jpg").transpose(Image.FLIP_TOP_BOTTOM)  # OpenGL origin bottom left
data = np.asarray(im, dtype=np.uint8)
f.write(struct.pack("2I", data.shape[1], data.shape[0]))
f.write(data.tobytes())
print(f'Texture earth_daytime in {data.shape[1]}x{data.shape[0]} with {data.shape[2]} channel(s) is written. 2 left')

im = Image.open("earth_night.jpg").transpose(Image.FLIP_TOP_BOTTOM)
data = np.asarray(im, dtype=np.uint8)
f.write(struct.pack("2I", data.shape[1], data.shape[0]))
f.write(data.tobytes())
print(f'Texture earth_night in {data.shape[1]}x{data.shape[0]} with {data.shape[2]} channel(s) is written. 1 left')

im = Image.open("earth_cloud.jpg").transpose(Image.FLIP_TOP_BOTTOM)
data = np.asarray(im, dtype=np.uint8)
f.write(struct.pack("2I", data.shape[1], data.shape[0]))
data = data[:,:,0]  # grayscale, drop 2 channels
f.write(data.tobytes())
print(f'Texture earth_cloud in {data.shape[1]}x{data.shape[0]} with 1 channel(s) is written.')

f.close()

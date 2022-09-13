"""
INSTRUCTIONS:
 Call from the commandline with the two images as the arguments.
"""

from PIL import Image
import math
import sys

def dist(v1, v2):
  (x,y,z,_) = v1
  (a,b,c,_) = v2
  return math.sqrt((x-a)*(x-a) + (y-b)*(y-b) + (z-c)*(z-c))

if __name__ == "__main__":
    print("File1: " + str(sys.argv[1]))
    img1 = Image.open(sys.argv[1]) # Can be many different formats.
    pix1 = img1.load()

    print("File2: " + str(sys.argv[2]))
    img2 = Image.open(sys.argv[2]) # Can be many different formats.
    pix2 = img2.load()

    avg_dist = 0

    for x in range(img1.size[0]):
      for y in range(img1.size[1]):
        avg_dist += dist(pix1[x,y], pix2[x,y]) / (img1.size[0]*img1.size[1])

    print("AVG distance:" + str(avg_dist))
    print("AVG distance (normalized):" + str(avg_dist/255.0))

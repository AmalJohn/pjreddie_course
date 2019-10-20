from uwimg import *

im = load_image("data/dog.jpg")
res = sobel_image(im)
mag = res[0]
direc = res[1]
feature_normalize(mag)
save_image(mag, "magnitude")
save_image(direc, "direction")

col_sobel = colorize_sobel(im)
save_image(col_sobel, "colorized-sobel")

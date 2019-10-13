from uwimg import *

im1 = load_image("data/aria.png")
im2 = load_image("data/melisa.png")
f = make_gaussian_filter(0.03)
lfreq = convolve_image(im2, f, 1)
melisaria = im1 + lfreq
clamp_image(melisaria)
save_image(melisaria, "melisaria")

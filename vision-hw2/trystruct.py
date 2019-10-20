from uwimg import *

im = load_image("data/dogbw.png")
s = structure_matrix(im, 2)
feature_normalize(s);
save_image(s, "dog-structmat")

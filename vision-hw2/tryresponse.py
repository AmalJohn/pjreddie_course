from uwimg import *

im = load_image("data/dogbw.png")
s = structure_matrix(im, 2)
r = cornerness_response(s)
feature_normalize(r);
save_image(r, "dog-response")
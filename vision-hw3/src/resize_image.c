#include <math.h>
#include "image.h"

float nn_interpolate(image im, float x, float y, int c)
{
    return get_pixel(im, round(x), round(y), c);
}

image nn_resize(image im, int w, int h)
{
	image resized_image = make_image(w, h, im.c);

	float a_x = (float)im.w / (float)w;
    float b_x = -0.5 + 0.5 * a_x;
    float a_y = (float)im.h / (float)h;
    float b_y = -0.5 + 0.5 * a_y;

	for (unsigned int i = 0; i < h; i++) {
		for (unsigned int j = 0; j < w; j++) {
			set_pixel(resized_image, j, i, 0, nn_interpolate(im, (a_x*j + b_x), (a_y*i + b_y), 0));
			set_pixel(resized_image, j, i, 1, nn_interpolate(im, (a_x*j + b_x), (a_y*i + b_y), 1));
			set_pixel(resized_image, j, i, 2, nn_interpolate(im, (a_x*j + b_x), (a_y*i + b_y), 2));
		}
	}
    return resized_image;
}

float bilinear_interpolate(image im, float x, float y, int c)
{
	float v1 = get_pixel(im, floorf(x), floorf(y), c);
   	float v2 = get_pixel(im, floorf(x), ceilf(y), c);
   	float v3 = get_pixel(im, ceilf(x), floorf(y), c);
   	float v4 = get_pixel(im, ceilf(x) + 1, ceilf(y), c);
   	float d1 = y - floorf(y);
   	float d2 = ceilf(y) - y;
   	float d3 = x - floorf(x);
   	float d4 = ceilf(x) - x;
   	float q1 = v1 * d2 + v2 * d1;
   	float q2 = v3 * d2 + v4 * d1;
   	return q1 * d4 + q2 * d3;
}

image bilinear_resize(image im, int w, int h)
{
    image resized_image = make_image(w, h, im.c);

	float a_x = (float)im.w / (float)w;
    float b_x = -0.5 + 0.5 * a_x;
    float a_y = (float)im.h / (float)h;
    float b_y = -0.5 + 0.5 * a_y;
	float newX, newY;

	for (unsigned int i = 0; i < h; i++) {
		for (unsigned int j = 0; j < w; j++) {
			newX = (a_x*j + b_x);
			newY = (a_y*i + b_y);

			set_pixel(resized_image, j, i, 0, bilinear_interpolate(im, newX, newY, 0));
			set_pixel(resized_image, j, i, 1, bilinear_interpolate(im, newX, newY, 1));
			set_pixel(resized_image, j, i, 2, bilinear_interpolate(im, newX, newY, 2));
		}
	}
    return resized_image;
}


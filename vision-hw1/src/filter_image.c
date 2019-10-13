#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

void l1_normalize(image im)
{
	float size = im.w * im.h;
	for (unsigned int i = 0; i < im.h; i++)
		for (unsigned int j = 0; j < im.w; j++) {
			set_pixel(im, j, i, 0, (1.0 / size));
		}
}

image make_box_filter(int w)
{
	image box_filter = make_image(w, w, 1);
	l1_normalize(box_filter);
    return box_filter;
}

image convolve_image(image im, image filter, int preserve)
{
	assert(im.c == filter.c || filter.c == 1);

	image convolved_image;
	int subst_w, subst_h;
	float sum;

	if (preserve) convolved_image = make_image(im.w, im.h, im.c);
	else convolved_image = make_image(im.w, im.h, 1);


	for (unsigned int c = 0; c < im.c; c++) {

		int filter_c = (filter.c==1) ? 0 : c;

		for (unsigned int i = 0; i < im.h; i++)
			for (unsigned int j = 0; j < im.w; j++) {
			
				sum = 0;
				subst_w = (filter.w / 2);
				subst_h = (filter.h / 2);
				for (unsigned int n = 0; n < filter.h; n++)
					for (unsigned int m = 0; m < filter.w; m++)
						sum += get_pixel(im, j + m - subst_w, i + n - subst_h, c) * get_pixel(filter, m, n, filter_c);
				
				if(preserve) {
                    set_pixel(convolved_image, j, i, c, sum);
				}
                else {
                    float p = get_pixel(convolved_image, j, i, 0);
                    set_pixel(convolved_image, j, i, 0, sum + p);
				}
			}
	}


    return convolved_image;
}

image make_highpass_filter()
{
    image filter = make_image(3,3,1);

    set_pixel(filter, 1, 0, 0, -1);
    set_pixel(filter, 0, 1, 0, -1);
    set_pixel(filter, 1, 1, 0, 4);
    set_pixel(filter, 2, 1, 0, -1);
    set_pixel(filter, 1, 2, 0, -1);

    return filter;
}

image make_sharpen_filter()
{
    image filter = make_image(3,3,1);

    set_pixel(filter, 1, 0, 0, -1);
    set_pixel(filter, 0, 1, 0, -1);
    set_pixel(filter, 1, 1, 0, 5);
    set_pixel(filter, 2, 1, 0, -1);
    set_pixel(filter, 1, 2, 0, -1);

    return filter;
}

image make_emboss_filter()
{

	image filter = make_image(3,3,1);

    set_pixel(filter, 0, 0, 0, -2);
    set_pixel(filter, 1, 0, 0, -1);
    set_pixel(filter, 2, 0, 0, 0);
    set_pixel(filter, 0, 1, 0, -1);
    set_pixel(filter, 1, 1, 0, 1);
    set_pixel(filter, 2, 1, 0, 1);
    set_pixel(filter, 0, 2, 0, 0);
    set_pixel(filter, 1, 2, 0, 1);
    set_pixel(filter, 2, 2, 0, 2);

    return filter;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: TODO

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: Yes, clamp_image() to change values which are not in (0,1) range

image make_gaussian_filter(float sigma)
{
    int size = round(sigma * 6);
    size = (size%2==1) ? size : size+1;

    image ga_f = make_image(size, size, 1);
    int centre = size/2;
    for(int y=0; y<ga_f.h; y++)
        for(int x=0; x<ga_f.w; x++) {
            float s2 = sigma*sigma;
            float r2 = (x-centre)*(x-centre) + (y-centre)*(y-centre);
            float v = 1.0/(TWOPI*s2) * exp(-r2/(2*s2));
            set_pixel(ga_f, x, y, 0, v);
        }

    l1_normalize(ga_f);
    return ga_f;
}

image add_image(image a, image b)
{
    image added_image = make_image(a.w, a.h, a.c);
	for (unsigned int h = 0; h < a.h; h++)
		for (unsigned int w = 0; w < a.w; w++) {
			set_pixel(added_image, w, h, 0, (get_pixel(a, w, h, 0) + get_pixel(b, w, h, 0)));
			set_pixel(added_image, w, h, 1, (get_pixel(a, w, h, 1) + get_pixel(b, w, h, 1)));
			set_pixel(added_image, w, h, 2, (get_pixel(a, w, h, 2) + get_pixel(b, w, h, 2)));
		}
	return added_image;
}

image sub_image(image a, image b)
{
    image subed_image = make_image(a.w, a.h, a.c);
	for (unsigned int h = 0; h < a.h; h++)
		for (unsigned int w = 0; w < a.w; w++) {
			set_pixel(subed_image, w, h, 0, (get_pixel(a, w, h, 0) - get_pixel(b, w, h, 0)));
			set_pixel(subed_image, w, h, 1, (get_pixel(a, w, h, 1) - get_pixel(b, w, h, 1)));
			set_pixel(subed_image, w, h, 2, (get_pixel(a, w, h, 2) - get_pixel(b, w, h, 2)));
		}
	return subed_image;
}

image make_gx_filter()
{
    image filter = make_image(3,3,1);

    set_pixel(filter, 0, 0, 0, -1);
    set_pixel(filter, 1, 0, 0, 0);
    set_pixel(filter, 2, 0, 0, 1);
    set_pixel(filter, 0, 1, 0, -2);
    set_pixel(filter, 1, 1, 0, 0);
    set_pixel(filter, 2, 1, 0, 2);
    set_pixel(filter, 0, 2, 0, -1);
    set_pixel(filter, 1, 2, 0, 0);
    set_pixel(filter, 2, 2, 0, 1);

    return filter;
}

image make_gy_filter()
{
    image filter = make_image(3,3,1);

    set_pixel(filter, 0, 0, 0, -1);
    set_pixel(filter, 1, 0, 0, -2);
    set_pixel(filter, 2, 0, 0, -1);
    set_pixel(filter, 0, 1, 0, 0);
    set_pixel(filter, 1, 1, 0, 0);
    set_pixel(filter, 2, 1, 0, 0);
    set_pixel(filter, 0, 2, 0, 1);
    set_pixel(filter, 1, 2, 0, 2);
    set_pixel(filter, 2, 2, 0, 1);

    return filter;
}

void feature_normalize(image im)
{
	float min = 1;
	float max = -1;
	float pixel;
    for (unsigned int c = 0; c < im.c; c++)
		for (unsigned int h = 0; h < im.h; h++)
			for (unsigned int w = 0; w < im.w; w++) {
				pixel = get_pixel(im, w, h, c);
				if (pixel < min) min = pixel;
				if (pixel > max) max = pixel;
			}
	
	float range = max - min;
	if (range == 0) {
		for (unsigned int c = 0; c < im.c; c++)
		for (unsigned int h = 0; h < im.h; h++)
			for (unsigned int w = 0; w < im.w; w++) {
				set_pixel(im, w, h, c, 0);
			}
	}

	for (unsigned int c = 0; c < im.c; c++)
		for (unsigned int h = 0; h < im.h; h++)
			for (unsigned int w = 0; w < im.w; w++) {
				pixel = get_pixel(im, w, h, c);
				set_pixel(im, w, h, c, (pixel - min) / range);
			}
}

image *sobel_image(image im)
{
	image *res = calloc(2, sizeof(image));
	image gx_filter = make_gx_filter();
	image gy_filter = make_gy_filter();
	image gx_conv = convolve_image(im, gx_filter, 0);
	image gy_conv = convolve_image(im, gy_filter, 0);
	
	image magnitude = make_image(gx_conv.w, gx_conv.h, gx_conv.c);
	image direction = make_image(gx_conv.w, gx_conv.h, gx_conv.c);

	float gx_pixel, gy_pixel, mag, dir;
	for (unsigned int h = 0; h < im.h; h++)
		for (unsigned int w = 0; w < im.w; w++) {
			gx_pixel = get_pixel(gx_conv, w, h, 0);
			gy_pixel = get_pixel(gy_conv, w, h, 0);
			mag = sqrt(gx_pixel*gx_pixel + gy_pixel*gy_pixel);
			dir = atan2(gx_pixel, gy_pixel);

			set_pixel(magnitude, w, h, 0, mag);
			if (isnan(dir))
                set_pixel(direction, w, h, 0, 0);
            else
				set_pixel(direction, w, h, 0, dir);
		}

	res[0] = magnitude;
	res[1] = direction;

    return res;
}

image colorize_sobel(image im)
{
	image col_sobel = make_image(im.w, im.h, im.c);

	image *sobel = sobel_image(im);
	feature_normalize(sobel[0]);

	float mag_pixel, dir_pixel;
	for (unsigned int h = 0; h < im.h; h++)
		for (unsigned int w = 0; w < im.w; w++) {
			mag_pixel = get_pixel(sobel[0], w, h, 0);
			dir_pixel = get_pixel(sobel[1], w, h, 0);
			set_pixel(col_sobel, w, h, 0, dir_pixel);
			set_pixel(col_sobel, w, h, 1, mag_pixel);
			set_pixel(col_sobel, w, h, 2, mag_pixel);
		}

	hsv_to_rgb(col_sobel);
	return col_sobel;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

void l1_normalize(image im) {
	float sum = 0;
	for (int c = 0; c < im.c; c++)
		for (int w = 0; w < im.w; w++)
			for (int h = 0; h < im.h; h++) {
				sum += get_pixel(im, w, h, c);
			}

	for (int c = 0; c < im.c; c++)
		for (int w = 0; w < im.w; w++)
			for (int h = 0; h < im.h; h++) {
				set_pixel(im, w, h, c, (get_pixel(im, w, h, c) / sum));
			}

}

image make_box_filter(int w)
{
	image box_filter = make_image(w, w, 1);
	float size = w*w;
	for (int i = 0; i < w; i++)
		for (int j = 0; j < w; j++) {
			set_pixel(box_filter, j, i, 0, (1.0 / size));
		}
    return box_filter;
}

image convolve_image(image im, image filter, int preserve)
{
	// TODO
    assert(im.c == filter.c || filter.c == 1);
    image conv_im;
    if (preserve == 1)
        conv_im = make_image(im.w, im.h, im.c);
    else
        conv_im = make_image(im.w, im.h, 1);
    for (int ch = 0; ch < im.c; ch++) {
        for (int y = 0; y < im.h; y++) {
            for (int x = 0; x < im.w; x++) {
                float sum = 0;
                for (int fy = 0; fy < filter.h; fy++) {
                    for (int fx = 0; fx < filter.w; fx++) {
                        int fc = 0;
                        if (im.c == filter.c) {
                            fc = ch;
                        }
                        sum += filter.data[fc * filter.h * filter.w + fy * filter.w + fx] *
                            get_pixel(im, x + fx - (filter.w / 2), y + fy - (filter.h / 2), ch);
                    }
                }
                if (preserve == 1)
                    conv_im.data[ch * conv_im.h * conv_im.w + y * conv_im.w + x] = sum;
                else
                    conv_im.data[y * conv_im.w + x] += sum;
            }
        }
    }
    // for (int y = 0; y < conv_im.h; y++) {
    //     for (int x = 0; x < conv_im.w; x++) {
    //         printf("conv_im: x %d, y %d: %f\n", x, y, get_pixel(conv_im, x, y, 1));
    //     }
    // }
    return conv_im;
}

image make_highpass_filter()
{
    image im = make_image(3, 3, 1);

    float filter[] = { 0, -1, 0, -1, 4, -1, 0, -1, 0};
    memcpy(im.data, filter, sizeof(filter));

    return im;
}

image make_sharpen_filter()
{
    image im = make_image(3, 3, 1);

    float filter[] = { 0, -1, 0, -1, 5, -1, 0, -1, 0};
    memcpy(im.data, filter, sizeof(filter));

    return im;
}

image make_emboss_filter()
{

	image im = make_image(3, 3, 1);

    float filter[] = { -2, -1, 0, -1, 1, 1, 0, 1, 2};
    memcpy(im.data, filter, sizeof(filter));

    return im;
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
    image im = make_image(3, 3, 1);

    float filter[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1};
    memcpy(im.data, filter, sizeof(filter));

    return im;
}

image make_gy_filter()
{
    image im = make_image(3, 3, 1);

    float filter[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1};
    memcpy(im.data, filter, sizeof(filter));

    return im;
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
        return;
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
			dir = atan(gy_pixel / gx_pixel);

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
    image mag = sobel[0];
    image dir = sobel[1];
	feature_normalize(mag);

	float mag_pixel, dir_pixel;
	for (unsigned int h = 0; h < im.h; h++)
		for (unsigned int w = 0; w < im.w; w++) {
			mag_pixel = get_pixel(mag, w, h, 0);
			dir_pixel = get_pixel(dir, w, h, 0);
			set_pixel(col_sobel, w, h, 0, dir_pixel);
			set_pixel(col_sobel, w, h, 1, mag_pixel);
			set_pixel(col_sobel, w, h, 2, mag_pixel);
		}

	hsv_to_rgb(col_sobel);

	return col_sobel;
}

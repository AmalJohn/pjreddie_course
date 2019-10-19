#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

float get_pixel(image im, int x, int y, int c)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > im.w) x = im.w - 1;
    if (y > im.h) y = im.h - 1;
    float pixel = im.data[im.w*im.h*c + im.w*y + x];
    return pixel;
}

void set_pixel(image im, int x, int y, int c, float v)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > im.w) x = im.w - 1;
    if (y > im.h) y = im.h - 1;
    im.data[im.w*im.h*c + im.w*y + x] = v;
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    memcpy(copy.data, im.data, im.w*im.h*im.c*sizeof(float));
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
    int im_size = im.w*im.h;
    for (unsigned int i = 0; i < im_size; i++) {
        // gray.data[i] = (im.data[i] + im.data[i + im_size] + im.data[i + im_size*2]) / 3; // Weighted mean K = (R+G+B)/3
        gray.data[i] = 0.299*im.data[i] + 0.587*im.data[i + im_size] + 0.114*im.data[i + im_size*2]; // Weighted sum of luma
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    int im_size = im.w*im.h;
	for (unsigned int i = 0; i < im_size; i++) {
		im.data[i + im_size*c] += v;
	}
}

void clamp_image(image im)
{
    int im_size = im.w*im.h*im.c;
	for (unsigned int i = 0; i < im_size; i++) {
		if (im.data[i] < 0) im.data[i] = 0;
		if (im.data[i] > 1) im.data[i] = 1;
	}
}


// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    for (int y = 0; y < im.h; y++)
    {
        for (int x = 0; x < im.w; x++)
        {
            float r = get_pixel(im, x, y, 0);
            float g = get_pixel(im, x, y, 1);
            float b = get_pixel(im, x, y, 2);

            float v = three_way_max(r, g, b);
            float m = three_way_min(r, g, b);

            float c = v - m;
            float s = 0;

            if (v > 0)
                s = c / v;

            float h_ = 0;
            if (c != 0)
            {
                if (v == r)
                    h_ = (g - b) / c;
                if (v == g)
                    h_ = (b - r) / c + 2;
                if (v == b)
                    h_ = (r - g) / c + 4;
            }
            float h = h_ / 6;
            if (h_ < 0)
                h = h_ / 6 + 1;

            set_pixel(im, x, y, 0, h);
            set_pixel(im, x, y, 1, s);
            set_pixel(im, x, y, 2, v);
        }
    }
}

void hsv_to_rgb(image im)
{
    for (int y = 0; y < im.h; y++)
    {
        for (int x = 0; x < im.w; x++)
        {
            float h = get_pixel(im, x, y, 0);
            float s = get_pixel(im, x, y, 1);
            float v = get_pixel(im, x, y, 2);

            float r = 0;
            float g = 0;
            float b = 0;

            if (s == 0)
            {
                r = 0;
                b = 0;
                g = 0;
            }

            float i = trunc(h * 6);
            float f = (h * 6) - i;
            float p = v * (1 - s);
            float q = v * (1 - s * f);
            float t = v * (1 - s * (1 - f));

            if (i == 0)
            {
                r = v;
                g = t;
                b = p;
            }

            if (i == 1)
            {
                r = q;
                g = v;
                b = p;
            }

            if (i == 2)
            {
                r = p;
                g = v;
                b = t;
            }

            if (i == 3)
            {
                r = p;
                g = q;
                b = v;
            }

            if (i == 4)
            {
                r = t;
                g = p;
                b = v;
            }

            if (i == 5)
            {
                r = v;
                g = p;
                b = q;
            }

            set_pixel(im, x, y, 0, r);
            set_pixel(im, x, y, 1, g);
            set_pixel(im, x, y, 2, b);
        }
    }
}

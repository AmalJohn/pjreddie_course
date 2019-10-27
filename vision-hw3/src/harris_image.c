#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"
#include <time.h>

// Frees an array of descriptors.
// descriptor *d: the array.
// int n: number of elements in array.
void free_descriptors(descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        free(d[i].data);
    }
    free(d);
}

// Create a feature descriptor for an index in an image.
// image im: source image.
// int i: index in image for the pixel we want to describe.
// returns: descriptor for that index.
descriptor describe_index(image im, int i)
{
    int w = 5;
    descriptor d;
    d.p.x = i%im.w;
    d.p.y = i/im.w;
    d.data = calloc(w*w*im.c, sizeof(float));
    d.n = w*w*im.c;
    int c, dx, dy;
    int count = 0;
    // If you want you can experiment with other descriptors
    // This subtracts the central value from neighbors
    // to compensate some for exposure/lighting changes.
    for(c = 0; c < im.c; ++c){
        float cval = im.data[c*im.w*im.h + i];
        for(dx = -w/2; dx < (w+1)/2; ++dx){
            for(dy = -w/2; dy < (w+1)/2; ++dy){
                float val = get_pixel(im, i%im.w+dx, i/im.w+dy, c);
                d.data[count++] = cval - val;
            }
        }
    }
    return d;
}

// Marks the spot of a point in an image.
// image im: image to mark.
// ponit p: spot to mark in the image.
void mark_spot(image im, point p)
{
    int x = p.x;
    int y = p.y;
    int i;
    for(i = -9; i < 10; ++i){
        set_pixel(im, x+i, y, 0, 1);
        set_pixel(im, x, y+i, 0, 1);
        set_pixel(im, x+i, y, 1, 0);
        set_pixel(im, x, y+i, 1, 0);
        set_pixel(im, x+i, y, 2, 1);
        set_pixel(im, x, y+i, 2, 1);
    }
}

// Marks corners denoted by an array of descriptors.
// image im: image to mark.
// descriptor *d: corners in the image.
// int n: number of descriptors to mark.
void mark_corners(image im, descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        mark_spot(im, d[i].p);
    }
}

// Creates a 1d Gaussian filter.
// float sigma: standard deviation of Gaussian.
// returns: single row image of the filter.
image make_1d_gaussian(float sigma)
{
    int size = round(sigma * 6);
    size = (size%2==1) ? size : size+1;

    image ga_f = make_image(size, 1, 1);
    int centre = size/2;
    for(int x=0; x<ga_f.w; x++) {
        float s2 = sigma*sigma;
        float r2 = (x-centre)*(x-centre);
        float v = 1.0/(TWOPI*s2) * exp(-r2/(2*s2));
        set_pixel(ga_f, x, 0, 0, v);
    }

    l1_normalize(ga_f);
    return ga_f;
}

image make_1d_gaussian_axis(float sigma, int axis)
{
    int size = round(sigma * 6);
    size = (size%2==1) ? size : size+1;

    image ga_f;
    if (axis == 0) {
        ga_f = make_image(size, 1, 1);
    }
    else {
        ga_f = make_image(1, size, 1);
    }
    int centre = size/2;
    for(int x = 0; x < size; x++) {
        float s2 = sigma*sigma;
        float r2 = (x-centre)*(x-centre);
        float v = 1.0/(TWOPI*s2) * exp(-r2/(2*s2));
        if(axis == 0) {
            set_pixel(ga_f, x, 0, 0, v);
        }
        else {
            set_pixel(ga_f, 0, x, 0, v);
        }
    }

    l1_normalize(ga_f);
    return ga_f;
}

// Smooths an image using separable Gaussian filter.
// image im: image to smooth.
// float sigma: std dev. for Gaussian.
// returns: smoothed image.
image smooth_image(image im, float sigma)
{
    image gauss_x = make_1d_gaussian_axis(sigma, 0);
    image gauss_y = make_1d_gaussian_axis(sigma, 1);
    image s = convolve_image(convolve_image(im, gauss_x, 1), gauss_y, 1);
    free_image(gauss_x);
    free_image(gauss_y);
    return s;
}

// Calculate the structure matrix of an image.
// image im: the input image.
// float sigma: std dev. to use for weighted sum.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          third channel is IxIy.
image structure_matrix(image im, float sigma)
{
    image S = make_image(im.w, im.h, 3);
    
    image gx_filter = make_gx_filter();
    image gy_filter = make_gy_filter();
	image Ix = convolve_image(im, gx_filter, 0);
	image Iy = convolve_image(im, gy_filter, 0);

	float Ix_pixel, Iy_pixel;
	for (int h = 0; h < S.h; h++)
		for (int w = 0; w < S.w; w++) {
			Ix_pixel = get_pixel(Ix, w, h, 0);
			Iy_pixel = get_pixel(Iy, w, h, 0);
            //if (Ix_pixel > 1 || Ix_pixel < 0 || Iy_pixel > 1 || Iy_pixel < 0) printf("True ");
			set_pixel(S, w, h, 0, (Ix_pixel*Ix_pixel));
			set_pixel(S, w, h, 1, (Iy_pixel*Iy_pixel));
			set_pixel(S, w, h, 2, (Ix_pixel*Iy_pixel));
		}

    S = smooth_image(S, sigma);

    free_image(Ix);
    free_image(Iy);
    return S;
}

// Estimate the cornerness of each pixel given a structure matrix S.
// image S: structure matrix for an image.
// returns: a response map of cornerness calculations.
image cornerness_response(image S)
{
    image R = make_image(S.w, S.h, 1);
    // We'll use formulation det(S) - alpha * trace(S)^2, alpha = .06.
    float alpha = 0.06f;
    float Ix, Iy, IxIy, det, trace, resp;
    for (int w = 0; w < R.w; w++)
        for (int h = 0; h < R.h; h++) {
            Ix = get_pixel(S, w, h, 0);
            Iy = get_pixel(S, w, h, 1);
            IxIy = get_pixel(S, w, h, 2);
            det = Ix*Iy - IxIy*IxIy;
            trace = Ix + Iy;
            resp = det - alpha * trace*trace;
            set_pixel(R, w, h, 0, resp);
        }
    return R;
}

// Perform non-max supression on an image of feature responses.
// image im: 1-channel image of feature responses.
// int w: distance to look for larger responses.
// returns: image with only local-maxima responses within w pixels.
image nms_image(image im, int w)
{
    image r = copy_image(im);
    // TODO: perform NMS on the response map.
    // for every pixel in the image:
    //     for neighbors within w:
    //         if neighbor response greater than pixel response:
    //             set response to be very low (I use -999999 [why not 0??])

    int side = (2*w + 1);
    int half = (w % 2 == 1) ? ((side - 1) / 2) : (side / 2);
    int isGreater = 0;
    float pixel, w_pixel;
    for (int x = 0; x < r.w; x++)
        for (int y = 0; y < r.h; y++) {

            isGreater = 0;
            for(int w_x = 0; w_x < side; w_x++) {
                for(int w_y = 0; w_y < side; w_y++) {
                    pixel = get_pixel(im, x, y, 0);
                    w_pixel = get_pixel(im, x + w_x - half, y + w_y - half, 0);
                    if (w_pixel > pixel) {
                        isGreater = 1;
                        set_pixel(r, x, y, 0, -999999);
                        break;
                    }
                }
                if(isGreater) break;
            }
        }
    return r;
}

// Perform harris corner detection and extract features from the corners.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
// int *n: pointer to number of corners detected, should fill in.
// returns: array of descriptors of the corners in the image.
descriptor *harris_corner_detector(image im, float sigma, float thresh, int nms, int *n)
{
    // Calculate structure matrix
    image S = structure_matrix(im, sigma);

    // Estimate cornerness
    image R = cornerness_response(S);

    // Run NMS on the responses
    image Rnms = nms_image(R, nms);


    //TODO: count number of responses over threshold
    int count = 0;

    int size_column = Rnms.w; 
    int size_row = Rnms.h;
    int size_channel = Rnms.c;
    float pixel = 0.0f;

    for(int k = 0; k < size_channel; ++k){
        for(int j = 0; j < size_row; ++j){
            for(int i = 0; i < size_column; ++i){

                pixel = get_pixel(Rnms, i, j, k);
                if (pixel > thresh){

                    count++;
                }

            }
        }
    }

    
    image *sobel = sobel_image(im);
    image grad_image = copy_image(sobel[1]);
    feature_normalize(grad_image);

    *n = count; // <- set *n equal to number of corners in image.
    descriptor *d = calloc(count, sizeof(descriptor));
    //TODO: fill in array *d with descriptors of corners, use describe_index.
    int descriptor_count = 0;

    for(int k = 0; k < size_channel; ++k){
        for(int j = 0; j < size_row; ++j){
            for(int i = 0; i < size_column; ++i){

                pixel = get_pixel(Rnms, i, j, k);
                if ((pixel > thresh) && (descriptor_count < count)){

                    d[descriptor_count] = describe_index(im, (i+(j*size_column)));

                    descriptor_count++;
                }

            }
        }
    }

    free_image(S);
    free_image(R);
    free_image(Rnms);
    free_image(grad_image);
    free(sobel);
    return d;
}

// Find and draw corners on an image.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
void detect_and_draw_corners(image im, float sigma, float thresh, int nms)
{
    int n = 0;
    descriptor *d = harris_corner_detector(im, sigma, thresh, nms, &n);
    mark_corners(im, d, n);
}

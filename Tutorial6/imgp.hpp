/*******************************************************
* File: imgp.hpp
*
* Description: includes and function prototypes for sobel video filter
*
* Author: Michael Noon and Addison Sandvik
*
********************************************************/
#ifndef _IMGP_H
#define _IMGP_H

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <arm_neon.h>

#include <iostream>
#include <list>
#include <pthread.h>
#include <papi.h>

#define NUM_NATIVE_EVENTS 24

// void *grayscale(cv::Mat* input, cv::Mat* output);
// void *sobel(cv::Mat* input, cv::Mat* output);
void *thread_proc(void* pkg);
void *grayscale(cv::Mat* in, cv::Mat *out, intptr_t start, intptr_t end);
void *sobel(cv::Mat* in, cv::Mat *out, intptr_t start, intptr_t end);
// void *grayscale(void* pkg);
// void *sobel(void* pkg);

#endif

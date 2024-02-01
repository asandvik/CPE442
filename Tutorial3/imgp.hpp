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

#include <iostream>
#include <list>

// void *grayscale(cv::Mat* input, cv::Mat* output);
// void *sobel(cv::Mat* input, cv::Mat* output);
void *grayscale(void* pkg);
void *sobel(void* pkg);

#endif

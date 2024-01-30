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

cv::Mat grayscale(cv::Mat* frame);

void sobel(cv::Mat* frame);

#endif

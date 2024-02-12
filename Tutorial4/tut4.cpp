/*********************************************************
* File: imgp.cpp
*
* Description: This program filters a video with the Sobel
				filter.
*
* Author: Michael Noon and Addison Sandvik
*
**********************************************************/
// #include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <arm_neon.h>

#include <iostream>
#include <list>

using namespace cv;

#define NUM_ROWS 8
#define NUM_COLS 8

void print_mat(Mat *matrix);
void populate(Mat *matrix);
void vpopulate(Mat *matrix);

int main(int argc, char **argv) {

	bool display = true;

	// create display window
	if (argc > 1) {
		namedWindow("CPU", WINDOW_NORMAL | WINDOW_GUI_NORMAL);
		display = true;
	}

	// buffers for processing
	Mat input_1ch(NUM_ROWS, NUM_COLS, CV_8UC1);
	Mat output_1ch(NUM_ROWS, NUM_COLS, CV_8UC1);
	Mat input_3ch(NUM_ROWS, NUM_COLS, CV_8UC3);
	Mat output_3ch(NUM_ROWS, NUM_COLS, CV_8UC3);

	vpopulate(&input_3ch);
	print_mat(&input_3ch);

	// display image
	if (argc > 1) {
		imshow("CPU", input_3ch);
		waitKey(0);
	}
}

void print_mat(Mat *matrix) {

	uint8_t *data = matrix->data;
	uint8_t num_pix = matrix->rows * matrix->cols;
	uint8_t num_data = num_pix * matrix->channels();

	if (matrix->channels() == 1) {
		for (int i = 0; i < num_data; i++) {
			printf("%03d ", *(data+i));
			if (i % matrix->cols == matrix->cols - 1) {
				printf("\n");
			}
		}
	}
	else if (matrix->channels() == 3) {

		for (int i = 0; i < num_data; i++) {
			printf("%03d ", *(data+i));
			if (i % 3 == 2) {
				printf("| ");
			}
			if (i % (matrix->cols * 3) == (matrix->cols * 3) - 1) {
				printf("\n");
			}
		}
	}
	else {
		printf("Can only print matrices with 1 or 3 channels\n");
	}
}

void populate(Mat *matrix) {

	uint8_t *data = matrix->data;
	uint8_t num_pix = matrix->rows * matrix->cols;
	uint8_t num_data = num_pix * matrix->channels();

	for (int i=0; i < num_data; i++) {
		*(data+i) = i;
	}
}

void vpopulate(Mat *matrix) {

	uint8_t *data = matrix->data;
	uint8_t num_pix = matrix->rows * matrix->cols;
	uint8_t num_data = num_pix * matrix->channels();

	uint8x16_t vector = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint8x16_t increment = {16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16};

	for (int i=0; i < num_data; i += 16) {
		vst1q_u8(data+i, vector);
		vector = vaddq_u8(vector, increment);
	}
}

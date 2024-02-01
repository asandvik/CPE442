/*********************************************************
* File: imgp.cpp
*
* Description: brief description of file purpose
*
* Author: Michael Noon and Addison Sandvik
*
**********************************************************/
#include "imgp.hpp"

using namespace cv;

typedef Point3_<uint8_t> Pixel;
typedef Point_<int32_t> ImPoint;

typedef struct Gray_Pkg {
	Mat *raw_frame; // make global?
	Mat *gray_frame;
	uint8_t start;
	uint8_t span;

} Gray_Pkg;

typedef struct Edge_Pkg {
	Mat *gray_frame;
	Mat *edge_frame;
	uint8_t start;
	uint8_t span;
} Edge_Pkg;

const int32_t Gx[] = {
	-1,  0,  1,
	-2,  0,  2,
	-1,  0,  1
};

const int32_t Gy[] = {
	-1, -2, -1,
	 0,  0,  0,
	 1,  2,  1
};

int main() {

	pthread_t thread1, thread2, thread3, thread4;
	int iret1, iret2, iret3, iret4;

	namedWindow("CPU", WINDOW_AUTOSIZE);
	setWindowProperty("CPU", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
	VideoCapture reader("A.mp4");

	double num_cols = reader.get(CAP_PROP_FRAME_WIDTH);
	double num_rows = reader.get(CAP_PROP_FRAME_HEIGHT);

	Mat raw_frame(num_rows, num_cols, CV_8UC3);
	Mat gray_frame(num_rows, num_cols, CV_8UC1);
	Mat edge_frame(num_rows, num_cols, CV_8UC1);

	Gray_Pkg gpkg1 = {&raw_frame, &gray_frame, 0, 4};
	Gray_Pkg gpkg2 = {&raw_frame, &gray_frame, 1, 4};
	Gray_Pkg gpkg3 = {&raw_frame, &gray_frame, 2, 4};
	Gray_Pkg gpkg4 = {&raw_frame, &gray_frame, 3, 4};

	Edge_Pkg epkg1 = {&gray_frame, &edge_frame, 0, 4};
	Edge_Pkg epkg2 = {&gray_frame, &edge_frame, 1, 4};
	Edge_Pkg epkg3 = {&gray_frame, &edge_frame, 2, 4};
	Edge_Pkg epkg4 = {&gray_frame, &edge_frame, 3, 4};

	for(;;) {

		if(!reader.read(raw_frame)) break;

		iret1 = pthread_create(&thread1, NULL, grayscale, &gpkg1);
		iret2 = pthread_create(&thread2, NULL, grayscale, &gpkg2);
		iret3 = pthread_create(&thread3, NULL, grayscale, &gpkg3);
		iret4 = pthread_create(&thread4, NULL, grayscale, &gpkg4);

		pthread_join(thread1, NULL);
     	pthread_join(thread2, NULL);
		pthread_join(thread3, NULL);
     	pthread_join(thread4, NULL); 

		iret1 = pthread_create(&thread1, NULL, sobel, &epkg1);
		iret2 = pthread_create(&thread2, NULL, sobel, &epkg2);
		iret3 = pthread_create(&thread3, NULL, sobel, &epkg3);
		iret4 = pthread_create(&thread4, NULL, sobel, &epkg4);

		pthread_join(thread1, NULL);
     	pthread_join(thread2, NULL);
		pthread_join(thread3, NULL);
     	pthread_join(thread4, NULL); 

		imshow("CPU", edge_frame);
		waitKey(1); // need delay for frame to show? 30 frames/sec -> delay 33 msec
	}
}

/*-----------------------------------------------------
* Function: grayscale
*
* Description: converts three-channel color matrix to 
* 	       one-channel grayscale following CCIR 601 
*
* param input: cv::Mat: address of color frame (3 channels)
* param output: cv::Mat: address of gray frame (1 channel)
*
* return: none
*--------------------------------------------------------*/
void *grayscale(void *pkg){
	Gray_Pkg *info = (Gray_Pkg*)pkg;

	Mat *raw_frame = info->raw_frame;
	Mat *gray_frame = info->gray_frame;

	uint32_t num_pix = raw_frame->rows * raw_frame->cols;
	
	for (int i = info->start; i < num_pix-1; i += info->span) {
		int row = i / raw_frame->cols;
		int col = i % raw_frame->cols;
	
		Pixel* ptr = raw_frame->ptr<Pixel>(row, col);
		uint8_t num = 0.299*ptr->x + 0.587*ptr->y + 0.114*ptr->z;
		uint8_t* opt = gray_frame->ptr<uint8_t>(row, col);
		*opt = num;
	}
	return NULL;
}

/*-----------------------------------------------------
* Function: sobel
*
* Description: filters grayscale matrix to perform edge detection 
*
* param input: cv::Mat: pointer to grayscale frame to be processed
* param output: cv::Mat: pointer to edge frame to store output
*
* return: NULL
*--------------------------------------------------------*/
void *sobel(void *pkg){

	Edge_Pkg *info = (Edge_Pkg*)pkg;

	Mat *gray_frame = info->gray_frame;
	Mat *edge_frame = info->edge_frame;

	uint32_t num_pix = gray_frame->rows * gray_frame->cols;
	
	for (int i = info->start; i < num_pix-1; i += info->span) {
		int row = i / gray_frame->cols;
		int col = i % gray_frame->cols;

		uint8_t neighborhood[] = {
		
			gray_frame->at<uint8_t>(row-1, col-1),	gray_frame->at<uint8_t>(row-1, col),	gray_frame->at<uint8_t>(row-1, col+1),
			gray_frame->at<uint8_t>(row, col-1),	gray_frame->at<uint8_t>(row, col),		gray_frame->at<uint8_t>(row, col+1),
			gray_frame->at<uint8_t>(row+1, col-1),  gray_frame->at<uint8_t>(row+1, col),	gray_frame->at<uint8_t>(row+1, col+1)

		};

		int32_t gx = 0;
		int32_t gy = 0;

		for(int i=0;i<9;i++){
			gx += neighborhood[i] * Gx[i];
			gy += neighborhood[i] * Gy[i];
		}
		
		int32_t gf =  abs(gx) + abs(gy);
		if(gf > 255) gf = 255;

		uint8_t* newpix = edge_frame->ptr<uint8_t>(row, col);
		*newpix = gf;
		
	}
	return NULL;
}

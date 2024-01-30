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

int main(){
        
	namedWindow("CPU", WINDOW_NORMAL);
	Mat frame;
	VideoCapture reader("A.mp4");
	resizeWindow("CPU", 1600, 900);
	
	int num = 0;
	
	for(;;){
		if(!reader.read(frame)) break;

		frame = grayscale(&frame);

		sobel(&frame);

		imshow("CPU", frame);
		waitKey(1); // need delay for frame to show? 30 frames/sec -> delay 33 msec
		// printf("%d\n", ++num);
	}


}

/*-----------------------------------------------------
* Function: grayscale
*
* Description: converts three-channel color matrix to 
* 	       one-channel grayscale following CCIR 601 
*
* param frame: cv::Mat: video frame to be processed
*
* return: cv::Mat: output one-channel video frame in grayscale
*--------------------------------------------------------*/
Mat grayscale(Mat* frame){
	Mat ret(frame->rows, frame->cols, CV_8UC1);
	for(int row=0;row<frame->rows; row++){
        	for(int col=0;col<frame->cols;col++){
			Pixel* ptr = frame->ptr<Pixel>(row, col);
			uint8_t num = 0.299*ptr->x + 0.587*ptr->y + 0.114*ptr->z;
			uint8_t* opt = ret.ptr<uint8_t>(row, col);
			*opt = num;
		}
	}
	return ret;
}

/*-----------------------------------------------------
* Function: sobel
*
* Description: filters 3-channel grayscale matrix to perform edge detection 
*	       (Note: matrix channel count unchanged)
*
* param frame: cv::Mat: video frame to be processed
*
* return: NULL
*--------------------------------------------------------*/
void sobel(Mat* frame){
	Mat framecopy = frame->clone();
	for(int row=0;row<frame->rows; row++){
		for(int col=0;col<frame->cols;col++){
			ImPoint cpoints[] = {
						
				ImPoint(row-1, col-1),	ImPoint(row-1, col),	ImPoint(row-1, col+1),
				ImPoint(row, col-1),	ImPoint(row, col),	ImPoint(row, col+1),
				ImPoint(row+1, col-1),  ImPoint(row+1, col),	ImPoint(row+1, col+1)

			};

			int32_t gx = 0;
			int32_t gy = 0;
			for(int i=0;i<9;i++){
				int32_t addx = 0;
				int32_t addy = 0;
				ImPoint cpt = cpoints[i];
				if(cpt.x >= 0 && cpt.x < frame->rows && cpt.y >= 0 && cpt.y < frame->cols){
					uint8_t* cpix = framecopy.ptr<uint8_t>(cpt.x, cpt.y);
					addx = *cpix * Gx[i];
					addy = *cpix * Gy[i];
				}
				gx += addx;
				gy += addy;	
			}
			
			int32_t gf =  abs(gx) + abs(gy);
			if(gf > 255) gf = 255;

			uint8_t* newpix = frame->ptr<uint8_t>(row, col);
			*newpix = gf;
		}
	}
}

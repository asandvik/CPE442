/*********************************************************
* File: imgp.cpp
*
* Description: This program filters a video with the Sobel
				filter.
*
* Author: Michael Noon and Addison Sandvik
*
**********************************************************/
#include "imgp.hpp"

using namespace cv;

typedef Point3_<uint8_t> Pixel;
typedef Point_<int32_t> ImPoint;

/* information package for grayscale() */
typedef struct Gray_Pkg {
	Mat *raw_frame; // input
	Mat *gray_frame; // output
	uint8_t rowstart;
	uint8_t rowend;

} Gray_Pkg;

/* information package for sobel() */
typedef struct Edge_Pkg {
	Mat *gray_frame; // input
	Mat *edge_frame; // output
	uint8_t start; // starting index
	uint8_t span; // increment to indices
} Edge_Pkg;

/* vertical edge Sobel filter */
const int32_t Gx[] = {
	-1,  0,  1,
	-2,  0,  2,
	-1,  0,  1
};

/* horizontal edge Sobel filter*/
const int32_t Gy[] = {
	-1, -2, -1,
	 0,  0,  0,
	 1,  2,  1
};

int main() {

	pthread_t thread1, thread2, thread3, thread4;
	int iret1, iret2, iret3, iret4;

	// create display window
	namedWindow("CPU", WINDOW_AUTOSIZE);
	setWindowProperty("CPU", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);

	// video frame reader
	VideoCapture reader("../Videos/A.mp4");
	double num_cols = reader.get(CAP_PROP_FRAME_WIDTH);
	double num_rows = reader.get(CAP_PROP_FRAME_HEIGHT);

	// buffers for processing
	Mat raw_frame(num_rows, num_cols, CV_8UC3);
	Mat gray_frame(num_rows, num_cols, CV_8UC1);
	Mat edge_frame(num_rows, num_cols, CV_8UC1);

	uint32_t row_chunk = num_rows >> 2; // divide by 4

	// information for threads
	Gray_Pkg gpkg1 = {&raw_frame, &gray_frame, 0, row_chunk};
	Gray_Pkg gpkg2 = {&raw_frame, &gray_frame, row_chunk, 2*row_chunk};
	Gray_Pkg gpkg3 = {&raw_frame, &gray_frame, 2*row_chunk, 3*row_chunk};
	Gray_Pkg gpkg4 = {&raw_frame, &gray_frame, 3*row_chunk, 4*row_chunk};

	Edge_Pkg epkg1 = {&gray_frame, &edge_frame, 0, 4};
	Edge_Pkg epkg2 = {&gray_frame, &edge_frame, 1, 4};
	Edge_Pkg epkg3 = {&gray_frame, &edge_frame, 2, 4};
	Edge_Pkg epkg4 = {&gray_frame, &edge_frame, 3, 4};

	for(;;) {

		// read frame
		if(!reader.read(raw_frame)) break;

		// launch grayscale threads
		iret1 = pthread_create(&thread1, NULL, grayscale, &gpkg1);
		iret2 = pthread_create(&thread2, NULL, grayscale, &gpkg2);
		iret3 = pthread_create(&thread3, NULL, grayscale, &gpkg3);
		iret4 = pthread_create(&thread4, NULL, grayscale, &gpkg4);

		// synchronize
		pthread_join(thread1, NULL);
     	pthread_join(thread2, NULL);
		pthread_join(thread3, NULL);
     	pthread_join(thread4, NULL); 

		// launch sobel threads
		iret1 = pthread_create(&thread1, NULL, sobel, &epkg1);
		iret2 = pthread_create(&thread2, NULL, sobel, &epkg2);
		iret3 = pthread_create(&thread3, NULL, sobel, &epkg3);
		iret4 = pthread_create(&thread4, NULL, sobel, &epkg4);

		// synchronize
		pthread_join(thread1, NULL);
     	pthread_join(thread2, NULL);
		pthread_join(thread3, NULL);
     	pthread_join(thread4, NULL); 

		// display
		imshow("CPU", edge_frame);
		waitKey(1);
	}
}

/*-----------------------------------------------------
* Function: grayscale
*
* Description: converts three-channel color matrix to 
* 	       one-channel grayscale following CCIR 601 
*
* param pkg: void*: Pointer to Gray_Pkg that contains:
* 		raw_frame: Mat*: address of color frame (input, 3 channels)
* 		gray_frame: Mat*: address of gray frame (output, 1 channel)
*		start: uint8_t: starting linear index
*		span: uint8_t: change to subsequent indices
*
* return: NULL
*--------------------------------------------------------*/
void *grayscale(void *pkg){
	Gray_Pkg *info = (Gray_Pkg*)pkg;

	Mat *raw_frame = info->raw_frame;
	Mat *gray_frame = info->gray_frame;
	uint32_t num_pix = raw_frame->rows * raw_frame->cols;

	uint8x16x3_t rgb_vec;
	uint8_t *in_vec_start = raw_frame->data;

	uint8_t *vr;
	uint8_t *vg;
	uint8_t *vb;

	for (i = 0; i < 14400; i++) {
		rgb_vec = vld3q_u8(in_vec_start);

		vstlq_u8(vr, rgb_vec.val[0]);
		vstlq_u8(vg, rgb_vec.val[1]);
		vstlq_u8(vb, rgb_vec.val[2]);

		

		/// working here

		in_vec_start += 48; // 3 * 16
	}

	for (int i = info->start; i < num_pix-1; i += info->span) {
		int row = i / raw_frame->cols;
		int col = i % raw_frame->cols;
	
		Pixel* ptr = raw_frame->ptr<Pixel>(row, col);
		uint8_t num = (4*ptr->x + 10*ptr->y + 2*ptr->z) >> 4;
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
* param pkg: void*: Pointer to Edge_Pkg that contains:
* 		gray_frame: Mat*: address of gray frame (input)
*		edge_frame: Mat*: address of edge frame (output)
*		start: uint8_t: starting linear index
*		span: uint8_t: change to subsequent indices
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

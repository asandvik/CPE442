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
	uint32_t start; // starting memory index
	uint32_t end; // ending memory index

} Gray_Pkg;

/* information package for sobel() */
typedef struct Edge_Pkg {
	Mat *gray_frame; // input
	Mat *edge_frame; // output
	uint32_t start; // starting memory index
	uint32_t end; // ending memory index
} Edge_Pkg;

/* vertical edge Sobel filter */
const int16x8_t Gx = {
	-1,  0,  1,
	-2,      2,
	-1,  0,  1
};

/* horizontal edge Sobel filter*/
const int16x8_t Gy = {
	-1, -2, -1,
	 0,      0,
	 1,  2,  1
};



int EventSet = PAPI_NULL;

void handle_papi_error(int retval){
	printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
	exit(1);
}

int main() {
	unsigned long_long tvalues[NUM_NATIVE_EVENTS];
	unsigned long_long gvalues[NUM_NATIVE_EVENTS];
	unsigned long_long svalues[NUM_NATIVE_EVENTS];
	unsigned long_long thvalues[NUM_NATIVE_EVENTS];
	unsigned long_long thvalues2[NUM_NATIVE_EVENTS];
	memset(tvalues, 0, NUM_NATIVE_EVENTS);
	memset(gvalues, 0, NUM_NATIVE_EVENTS);
	memset(svalues, 0, NUM_NATIVE_EVENTS);
	memset(thvalues, 0, NUM_NATIVE_EVENTS);
	memset(thvalues2, 0, NUM_NATIVE_EVENTS);

	int papi_ret, papi_ret2;
	//int EventSet2 = PAPI_NULL;
	int native = 0x0;
	PAPI_event_info_t info;
	std::vector<std::string> names{};

	papi_ret = PAPI_library_init(PAPI_VER_CURRENT);
	if(papi_ret != PAPI_VER_CURRENT){
		printf("PAPI Init Err!!\n");
		exit(1);
	}

	papi_ret = PAPI_create_eventset(&EventSet);
	if(papi_ret != PAPI_OK) handle_papi_error(papi_ret);
	//int counter = 0;
	native = PAPI_NATIVE_MASK | 0;
	papi_ret = PAPI_enum_event(&native, PAPI_ENUM_FIRST);
	while(papi_ret == PAPI_OK){
		if(PAPI_get_event_info(native, &info) == PAPI_OK){
			PAPI_add_event(EventSet, native);
			names.push_back(std::string{info.symbol});
		}
		papi_ret = PAPI_enum_event(&native, PAPI_ENUM_EVENTS);
	}
	int z = 0;

	PAPI_start(EventSet);
	//figure out how PAPI read works

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

	uint32_t chunk = (uint32_t)(num_rows * num_cols) >> 2; // divide by 4

	// information for threads
	Gray_Pkg gpkg1 = {&raw_frame, &gray_frame, 0, chunk};
	Gray_Pkg gpkg2 = {&raw_frame, &gray_frame, chunk, 2*chunk};
	Gray_Pkg gpkg3 = {&raw_frame, &gray_frame, 2*chunk, 3*chunk};
	Gray_Pkg gpkg4 = {&raw_frame, &gray_frame, 3*chunk, 4*chunk};

	Edge_Pkg epkg1 = {&gray_frame, &edge_frame, 0, chunk};
	Edge_Pkg epkg2 = {&gray_frame, &edge_frame, chunk, 2*chunk};
	Edge_Pkg epkg3 = {&gray_frame, &edge_frame, 2*chunk, 3*chunk};
	Edge_Pkg epkg4 = {&gray_frame, &edge_frame, 3*chunk, 4*chunk};

	for(;;) {

		// read frame
		if(!reader.read(raw_frame)) break;

		// launch grayscale threads
		iret1 = pthread_create(&thread1, NULL, grayscale, &gpkg1);
		iret2 = pthread_create(&thread2, NULL, grayscale, &gpkg2);
		iret3 = pthread_create(&thread3, NULL, grayscale, &gpkg3);
		iret4 = pthread_create(&thread4, NULL, grayscale, &gpkg4);
		PAPI_accum(EventSet, (long_long*)thvalues);
		// synchronize
		pthread_join(thread1, NULL);
     	pthread_join(thread2, NULL);
		pthread_join(thread3, NULL);
     	pthread_join(thread4, NULL); 
		PAPI_accum(EventSet, (long_long*)gvalues);
		// launch sobel threads
		iret1 = pthread_create(&thread1, NULL, sobel, &epkg1);
		iret2 = pthread_create(&thread2, NULL, sobel, &epkg2);
		iret3 = pthread_create(&thread3, NULL, sobel, &epkg3);
		iret4 = pthread_create(&thread4, NULL, sobel, &epkg4);
		PAPI_accum(EventSet, (long_long*)thvalues2);
		// synchronize
		pthread_join(thread1, NULL);
     	pthread_join(thread2, NULL);
		pthread_join(thread3, NULL);
     	pthread_join(thread4, NULL); 
		PAPI_accum(EventSet, (long_long*)svalues);

		// display
		imshow("CPU", edge_frame);
		waitKey(1);
	}

	std::cout.width(50); std::cout << std::left << "Name";
	std::cout.width(16); std::cout << std::left << "Grayscale Ratio";
	std::cout.width(12); std::cout << std::left << "Sobel Ratio";
	std::cout.width(19); std::cout << std::left << "Threading Overhead";
	std::cout.width(20); std::cout << std::left << "Total Counter Value" << std::endl;
	for(int i=0;i<NUM_NATIVE_EVENTS;i++){
		unsigned long_long tot = thvalues2[i] + thvalues[i] + gvalues[i] + svalues[i];
		std::cout.width(50); std::cout << std::left << names.at(i).c_str();
		char grat[11];
		char srat[11];
		char trat[11];
		if(tot){
			sprintf(grat, "%f", (double)gvalues[i]/tot);
			sprintf(srat, "%f", (double)svalues[i]/tot);
			sprintf(trat, "%f", (double)(thvalues[i]+thvalues2[i])/tot);
		}
		else{
			sprintf(grat, "NULL");
			sprintf(srat, "NULL");
			sprintf(trat, "NULL");
		}
		std::cout.width(16); std::cout << std::left << grat;
		std::cout.width(12); std::cout << std::left << srat;
		std::cout.width(19); std::cout << std::left << trat;
		std::cout.width(20); std::cout << std::left << tot << std::endl;

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

	uint8_t *input_start = raw_frame->data;
	uint8_t *output_start = gray_frame->data;

	for (int i = info->start; i < info->end; i += 8) {
		// load with deinterleave
		uint8x8x3_t rgb_vec = vld3_u8(input_start+i*3);

		// widen into 16 bit fields
		uint16x8_t vr = vmovl_u8(rgb_vec.val[0]);
		uint16x8_t vg = vmovl_u8(rgb_vec.val[1]);
		uint16x8_t vb = vmovl_u8(rgb_vec.val[2]);

		// multiply
		vr = vmulq_n_u16(vr, 4);
		vg = vmulq_n_u16(vg, 10);
		vb = vmulq_n_u16(vb, 2);

		// add channels together
		uint16x8_t vgray16 = vaddq_u16(vr, vg);
		vgray16 = vaddq_u16(vgray16, vb);

		// divide by 16
		vgray16 = vshrq_n_u16(vgray16, 4);

		// make 8 bit
		uint8x8_t vgray8 = vmovn_u16(vgray16);

		// write to output
		vst1_u8(output_start+i, vgray8);
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
	
	for (int i = info->start; i < info->end; i += 1) {
		int row = i / gray_frame->cols;
		int col = i % gray_frame->cols;

		uint8x8_t oner = vld1_u8(gray_frame->ptr<uint8_t>(row-1, col-1));
		uint8x8_t twor = vld1_u8(gray_frame->ptr<uint8_t>(row, col-1));
		uint8x8_t threer = vld1_u8(gray_frame->ptr<uint8_t>(row+1, col-1));
		int16x8_t neighborhood = {
		
			(int16_t)oner[0],	(int16_t)oner[1],	(int16_t)oner[2],
			(int16_t)twor[0],		(int16_t)twor[2],
			(int16_t)threer[0],  (int16_t)threer[1],	(int16_t)threer[2]

		};

		int16x8_t gxv = vmulq_s16(neighborhood, Gx);
		int16x8_t gyv = vmulq_s16(neighborhood, Gy);

		int16_t gx = vaddvq_s16(gxv);
		int16_t gy = vaddvq_s16(gyv);
		
		int16_t gf =  abs(gx) + abs(gy);
		if(gf > 255) gf = 255;

		uint8_t* newpix = edge_frame->ptr<uint8_t>(row, col);
		*newpix = (uint8_t)gf;
		
	}
	return NULL;
}



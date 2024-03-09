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

pthread_barrier_t gray_barrier;
pthread_barrier_t sobel_barrier;

typedef struct Pkg {
	Mat *raw_frame;
	Mat *gray_frame;
	Mat *edge_frame; 
	uint32_t start; // starting memory index
	uint32_t end; // ending memory index
} Pkg;

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

int run = 1;

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

	int papi_ret;
	int native;
	PAPI_event_info_t info;
	std::vector<std::string> names{};

	//init PAPI
	papi_ret = PAPI_library_init(PAPI_VER_CURRENT);
	if(papi_ret != PAPI_VER_CURRENT){
		printf("PAPI Init Err!!\n");
		exit(1);
	}

	//create event set
	papi_ret = PAPI_create_eventset(&EventSet);
	if(papi_ret != PAPI_OK) handle_papi_error(papi_ret);

	//get first native event
	native = PAPI_NATIVE_MASK | 0;
	papi_ret = PAPI_enum_event(&native, PAPI_ENUM_FIRST);

	//while the queried event exists
	while(papi_ret == PAPI_OK){

		//attempt to gather info, if info exists add the event to the set
		if(PAPI_get_event_info(native, &info) == PAPI_OK){
			if(PAPI_add_event(EventSet, native) == PAPI_OK){ 
				names.push_back(std::string{info.symbol});
			}
		}

		//query next native event
		papi_ret = PAPI_enum_event(&native, PAPI_ENUM_EVENTS);
	}
	
	//begin counting
	PAPI_start(EventSet);

	pthread_t thread1, thread2, thread3, thread4;
	int iret1, iret2, iret3, iret4;

	// create display window
	namedWindow("CPU", WINDOW_AUTOSIZE);
	// setWindowProperty("CPU", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);

	// video frame reader
	VideoCapture reader("../Videos/A.mp4");
	double num_cols = reader.get(CAP_PROP_FRAME_WIDTH);
	double num_rows = reader.get(CAP_PROP_FRAME_HEIGHT);

	// buffers for processing
	Mat raw_frame(num_rows, num_cols, CV_8UC3);
	Mat gray_frame(num_rows, num_cols, CV_8UC1);
	Mat edge_frame(num_rows, num_cols, CV_8UC1);

	uint32_t chunk = (uint32_t)(num_rows * num_cols) >> 2; // divide by 4

	Pkg pkg1 = {&raw_frame, &gray_frame, &edge_frame, 0, chunk};
	Pkg pkg2 = {&raw_frame, &gray_frame, &edge_frame, chunk, 2*chunk};
	Pkg pkg3 = {&raw_frame, &gray_frame, &edge_frame, 2*chunk, 3*chunk};
	Pkg pkg4 = {&raw_frame, &gray_frame, &edge_frame, 3*chunk, 4*chunk};

	pthread_barrier_init(&gray_barrier,NULL,5);
	pthread_barrier_init(&sobel_barrier,NULL,5);

	// read first frame
	reader.read(raw_frame);

	// launch threads
	iret1 = pthread_create(&thread1, NULL, thread_proc, &pkg1);
	iret2 = pthread_create(&thread2, NULL, thread_proc, &pkg2);
	iret3 = pthread_create(&thread3, NULL, thread_proc, &pkg3);
	iret4 = pthread_create(&thread4, NULL, thread_proc, &pkg4);

	while (run) {

		// wait for grayscale to complete
		pthread_barrier_wait(&gray_barrier);

		// preemtively load next frame
		if(!reader.read(raw_frame)) run = 0;

		// wait for sobel to complete
		pthread_barrier_wait(&sobel_barrier);

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

void *thread_proc2(void *pkg) {
	Pkg *info = (Pkg *)pkg;

	uint16_t cols = info->raw_frame->cols;
	uint16_t rows = info->raw_frame->rows;
	uint32_t num_pix = rows * cols;
	
	uint8_t *data_raw = info->raw_frame->data;
	uint8_t *data_gray = info->gray_frame->data;
	uint8_t *data_edge = info->edge_frame->data;

	uint32_t istart = info->start;
	uint32_t iend = info->end;

	if (istart == 0) istart = cols; // start at second row if first block
	if (iend == num_pix) iend = num_pix - cols; // stop before last row if last block

	while (run) {

		// Grayscale
		for (int i = istart; i < iend; i += 8) {
			// load with deinterleave
			uint8x8x3_t rgb_vec = vld3_u8(data_raw+i*3);

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
			vst1_u8(data_gray+i, vgray8);
		}

		pthread_barrier_wait(&gray_barrier);

		// Sobel
		for (int i = istart; i < iend; i += 1) {

			int16x8_t neighborhood = {
				(int16_t)data_gray[i-cols-1],	
				(int16_t)data_gray[i-cols],	
				(int16_t)data_gray[i-cols+1],

				(int16_t)data_gray[i-1],	
				(int16_t)data_gray[i+1],

				(int16_t)data_gray[i+cols-1],	
				(int16_t)data_gray[i+cols],	
				(int16_t)data_gray[i+cols+1],
			};

			int16x8_t gxv = vmulq_s16(neighborhood, Gx);
			int16x8_t gyv = vmulq_s16(neighborhood, Gy);

			int16_t gx = vaddvq_s16(gxv);
			int16_t gy = vaddvq_s16(gyv);
			
			int16_t gf =  abs(gx) + abs(gy);
			if(gf > 255) gf = 255;

			data_edge[i] = (uint8_t)gf;
		}

		pthread_barrier_wait(&sobel_barrier);
	}
	return NULL;
}

void *thread_proc(void *pkg) {
	Pkg *info = (Pkg *)pkg;

	while (run) {
		grayscale(info->raw_frame, info->gray_frame, info->start, info->end);
		pthread_barrier_wait(&gray_barrier);
		sobel(info->gray_frame, info->edge_frame, info->start, info->end);
		pthread_barrier_wait(&sobel_barrier);
	}
	return NULL;
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
void *grayscale(Mat *in, Mat *out, intptr_t istart, intptr_t iend){

	uint32_t num_pix = in->rows * in->cols;

	uint8_t *input_start = in->data;
	uint8_t *output_start = out->data;

	for (int i = istart; i < iend; i += 8) {
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
void *sobel(Mat *in, Mat *out, intptr_t istart, intptr_t iend){

	uint16_t cols = in->cols;
	uint16_t rows = in->rows;
	uint32_t num_pix = rows * cols;
	
	uint8_t *data_in = in->data;
	uint8_t *data_out = out->data;

	if (istart == 0) istart = cols; // start at second row if first block
	if (iend == num_pix) iend = num_pix - cols; // stop before last row if last block
	
	for (int i = istart; i < iend; i += 1) {

		int16x8_t neighborhood = {
			(int16_t)data_in[i-cols-1],	
			(int16_t)data_in[i-cols],	
			(int16_t)data_in[i-cols+1],

			(int16_t)data_in[i-1],	
			(int16_t)data_in[i+1],

			(int16_t)data_in[i+cols-1],	
			(int16_t)data_in[i+cols],	
			(int16_t)data_in[i+cols+1],
		};

		int16x8_t gxv = vmulq_s16(neighborhood, Gx);
		int16x8_t gyv = vmulq_s16(neighborhood, Gy);

		int16_t gx = vaddvq_s16(gxv);
		int16_t gy = vaddvq_s16(gyv);
		
		int16_t gf =  abs(gx) + abs(gy);
		if(gf > 255) gf = 255;

		data_out[i] = (uint8_t)gf;
	}
	return NULL;
}



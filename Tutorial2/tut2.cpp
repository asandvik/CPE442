/*********************************************************
* File: tut2.cpp
*
* Description: This program displays a frame from a video file
*
* Author: Michael Noon and Addison Sandvik
*
**********************************************************/
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#define WINDDOW_NAME "My Window"

using namespace cv;

int main() {

	namedWindow(WINDDOW_NAME, WINDOW_NORMAL); // Window to display our video frame
	resizeWindow(WINDDOW_NAME, 1280, 720); // resize window
	Mat frame; // Matrix to hold frame
	VideoCapture reader("thoop.mp4"); // Video reader
	
	if (reader.read(frame)) { // Read next frame from video. Returns false if no frame is read
		imshow(WINDDOW_NAME, frame); // Display frame in window
		waitKey(5000); // Wait five seconds
	} 

	// Deallocate window resources. This isn't really necessary here as the operating
	// system will handle deallocation when the program exits, but this may be important 
	// in more complex programs.
	destroyWindow(WINDDOW_NAME);

	return 0;
}
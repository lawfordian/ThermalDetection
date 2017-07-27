#include "HitDetector.h"

#include <iostream>

#include "opencv2/opencv.hpp"
#include "opencv2/bgsegm.hpp"

using namespace cv;

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240

// background subtractor params
#define BG_HISTORY 20
#define BG_NMIXTURES 5
#define BG_RATIO 1
#define BG_NOISE 0.7

// blob params
#define MIN_THRESH 1
#define MAX_THRESH 50
#define MIN_AREA 1
#define MAX_AREA 100
#define MIN_CIRCULARITY 0.01


HitDetector::HitDetector(void) {	
	//create background subtractor objects
	pMOG = bgsegm::createBackgroundSubtractorMOG(); //MOG approach
	//pMOG2 = bgsegm::createBackgroundSubtractorMOG(BG_HISTORY, BG_NMIXTURES, BG_RATIO, BG_NOISE); //MOG2 approach	
	// can use GMG, KNN or CNT additionally
	
	//create GUI windows
	namedWindow("FG Mask MOG", WINDOW_NORMAL);
	resizeWindow("FG Mask MOG", WINDOW_WIDTH, WINDOW_HEIGHT);
	//namedWindow("FG Mask MOG 2", WINDOW_NORMAL);
	//resizeWindow("FG Mask MOG 2", WINDOW_WIDTH, WINDOW_HEIGHT);
	namedWindow("keypoints", WINDOW_NORMAL);
	resizeWindow("keypoints", WINDOW_WIDTH, WINDOW_HEIGHT);
	
	//create blob detector objects
	SimpleBlobDetector::Params blobParam;
	blobParam.minThreshold = MIN_THRESH;
	blobParam.maxThreshold = MAX_THRESH;
	blobParam.filterByArea = true;
	blobParam.minArea = MIN_AREA;
	blobParam.maxArea = MAX_AREA;
	blobParam.filterByCircularity = true;
	blobParam.minCircularity = MIN_CIRCULARITY;
	blobDetector = SimpleBlobDetector::create(blobParam);
	
}

HitDetector::~HitDetector(void) {
}

void HitDetector::detectHit(Mat img) {
	
	Mat foregroundImg = backgroundSubtract(img);
	blobDetect(foregroundImg);
	
}

Mat HitDetector::backgroundSubtract(Mat inputImg) {
	
	Mat fgMaskMOG; //fg mask generated by MOG method
	//Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
	
	//update the background model
    pMOG->apply(inputImg, fgMaskMOG);
    //pMOG2->apply(inputImg, fgMaskMOG2);
	
	//show the current frame and the fg masks
    imshow("FG Mask MOG", fgMaskMOG);
    //imshow("FG Mask MOG 2", fgMaskMOG2);
	
	//return one of the two method results
	return fgMaskMOG;
	
}

void HitDetector::blobDetect(Mat image) {
	
	// Detect blobs.
	std::vector<KeyPoint> keypoints;
	blobDetector->detect(image, keypoints);
	
	for(uint i=0; i<keypoints.size(); i++) {
		printf("Hit detected, coordinates:\n");
		printf("%s %f %s %f\n", "xCoord = ", keypoints[i].pt.x, ", yCoord = ", keypoints[i].pt.y);
	}
	 
	// Draw detected blobs as red circles.
	// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
	Mat im_with_keypoints;
	drawKeypoints(image, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	 
	// Show blobs
	imshow("keypoints", im_with_keypoints);
	//waitKey(0);
	
}
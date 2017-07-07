#ifndef HITDETECTOR_H
#define HITDETECTOR_H

#include <stdint.h>

class HitDetector
{

public:
	
	HitDetector();
	~hitDetector();
	
	void detectHit(Mat img);

private:

	Ptr<BackgroundSubtractor> pMOG; 
	Ptr<BackgroundSubtractor> pMOG2; 
	Ptr<SimpleBlobDetector> blobDetector;
	
	Mat backgroundSubtract(Mat inputImg);
	void blobDetect(Mat image);
	
};

#endif

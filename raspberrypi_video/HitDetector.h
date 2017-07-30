#include "opencv2/opencv.hpp"

using namespace cv;

class HitDetector {
	public:
		void detectHit(Mat img);
		HitDetector();
		~HitDetector();

	private:
		Mat backgroundSubtract(Mat inputImg);
		void blobDetect(Mat image);
		Ptr<BackgroundSubtractor> pMOG;
		Ptr<BackgroundSubtractorMOG2> pMOG2;
		Ptr<SimpleBlobDetector> blobDetector;

};
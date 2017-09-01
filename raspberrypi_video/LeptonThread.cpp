#include "LeptonThread.h"

#include "Palettes.h"
#include "SPI.h"
#include "Lepton_I2C.h"
#include "HitDetector.cpp"

#include "opencv2/opencv.hpp"
#include <highgui.hpp>

#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 27
#define HEIGHT 60
#define WIDTH 80

#define API_PREF cv::CAP_V4L
#define VID_OUT_FILE "ThermalVisionVideo.avi"
#define VID_FPS FPS
#define VID_COL true

#define IMAGE_FORMAT QImage::Format_RGB888

// scene min & max
// 14bit number, we want a range roughly 10C -> 100C
#define MIN_DISP 7800
#define MAX_DISP 10000

LeptonThread::LeptonThread() : QThread()
{
}

LeptonThread::~LeptonThread() {
}

cv::Mat LeptonThread::QImage2Mat(QImage const& inImage) {
     	QImage   swapped = inImage;
        if ( inImage.format() == QImage::Format_RGB32 ) {
            swapped = swapped.convertToFormat( QImage::Format_RGB888 );
        }
        swapped = swapped.rgbSwapped();
        return cv::Mat( swapped.height(), swapped.width(),
        	CV_8UC3,
                const_cast<uchar*>(swapped.bits()),
                static_cast<size_t>(swapped.bytesPerLine())).clone();
}

void LeptonThread::run()
{
	//create the initial image
	myImage = QImage(WIDTH, HEIGHT, IMAGE_FORMAT);
	
	//create instance of the detector
	HitDetector *myHitDetector = new HitDetector();
	
	//create video writer
	cv::Size S = cv::Size(WIDTH, HEIGHT);
	cv::VideoWriter outputVideo = cv::VideoWriter::VideoWriter(VID_OUT_FILE, API_PREF, cv::VideoWriter::fourcc('H','2','6','4'), VID_FPS, S, VID_COL);
	if(outputVideo.isOpened()) {
		std::cout << "VideoWriter Opened.";
	}
	else {
		std::cout << "VideoWriter Failed.";
	}
	
	//open spi port
	SpiOpenPort(0);
	
	std::cout << "agc state: ";
	lepton_get_agc();
	
	std::cout << "aux temp: ";
	lepton_aux_temp();

	while(true) {

		//read data packets from lepton over SPI
		int resets = 0;
		for(int j=0;j<PACKETS_PER_FRAME;j++) {
			//if it's a drop packet, reset j to 0, set to -1 so he'll be at 0 again loop
			read(spi_cs0_fd, result+sizeof(uint8_t)*PACKET_SIZE*j, sizeof(uint8_t)*PACKET_SIZE);
			int packetNumber = result[j*PACKET_SIZE+1];
			if(packetNumber != j) {
				j = -1;
				resets += 1;
				usleep(1000);
				//Note: we've selected 750 resets as an arbitrary limit, since there should never be 750 "null" packets between two valid transmissions at the current poll rate
				//By polling faster, developers may easily exceed this count, and the down period between frames may then be flagged as a loss of sync
				if(resets == 750) {
					SpiClosePort(0);
					usleep(750000);
					SpiOpenPort(0);
				}
			}
		}
		if(resets >= 30) {
			qDebug() << "done reading, resets: " << resets;
		}

		frameBuffer = (uint16_t *)result;
		int row, column;
		uint16_t value;
		uint16_t minValue = MIN_DISP;
		uint16_t maxValue = MAX_DISP;

		
		for(int i=0;i<FRAME_SIZE_UINT16;i++) {
			//skip the first 2 uint16_t's of every packet, they're 4 header bytes
			if(i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
			
			//flip the MSB and LSB at the last second
			int temp = result[i*2];
			result[i*2] = result[i*2+1];
			result[i*2+1] = temp;
			
			value = frameBuffer[i];
			/* if(value > maxValue) {
				maxValue = value;
			}
			if(value < minValue) {
				minValue = value;
			} */
			column = i % PACKET_SIZE_UINT16 - 2;
			row = i / PACKET_SIZE_UINT16 ;
		}

		float diff = maxValue - minValue;
		float scale = 255/diff;
		QRgb color;
		for(int i=0;i<FRAME_SIZE_UINT16;i++) {
			if(i % PACKET_SIZE_UINT16 < 2) {
				continue;
			}
			value = (frameBuffer[i] - minValue) * scale;
			const int *colormap = colormap_grayscale;
			color = qRgb(colormap[3*value], colormap[3*value+1], colormap[3*value+2]);
			column = (i % PACKET_SIZE_UINT16 ) - 2;
			row = i / PACKET_SIZE_UINT16;
			myImage.setPixel(column, row, color);
		}

		//lets emit the signal for update
		emit updateImage(myImage);
		
		//convert image from QImage to Mat
		cv::Mat imageMat = QImage2Mat(myImage);
		
		//send image to hitDetector
		myHitDetector->detectHit(imageMat);
		
		//deal with video encoding
		outputVideo.write(imageMat);
	}
	// destroy videowriter
	outputVideo.release();
	
	//finally, close SPI port just bcuz
	SpiClosePort(0);
}

void LeptonThread::performFFC() {
	//perform FFC
	lepton_perform_ffc();
}

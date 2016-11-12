#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "SpecImage.h"

using namespace cv;
using namespace std;

// This method takes a given OpenCV Mat type from matrix.getType() and
//  returns the string representatiton of that type. Useful for debugging
//  purposes (eg. determining the bit depth of TIF images).
string type2str(int type) 
{
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) 
	{
		case CV_8U:  r = "8U"; break;
		case CV_8S:  r = "8S"; break;
		case CV_16U: r = "16U"; break;
		case CV_16S: r = "16S"; break;
		case CV_32S: r = "32S"; break;
		case CV_32F: r = "32F"; break;
		case CV_64F: r = "64F"; break;
		default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

int main(int argc, char* argv[])
{	
	SpecImage newImg("EO1H0420342016268110PF_1T");

	//Mat newRGB = newImg.getRGB();
	//imshow("RGB", newRGB);
	//imwrite("RGB.png", newRGB);
	//waitKey(0);

	Mat colorComposite = newImg.getComposite(641, 580, 509); // Hyperion reccomended color composite
	Mat swir = newImg.getComposite(1954, 1629, 1074); // Short Wavelength InfraRed (SWIR)

	Mat grayscale;
	Mat veg = newImg.getImage(855); // 16US1

	short arbitraryThreshold = 32768 / 12;
	veg.convertTo(veg, CV_8UC1, 255.0 / arbitraryThreshold);

	cvtColor(colorComposite, grayscale, CV_RGB2GRAY); // Convert to gray

	Mat redVegetation = grayscale.clone();
	cvtColor(redVegetation, redVegetation, CV_GRAY2BGR); // Return to 3 channels
	
	for (int r = 0; r < redVegetation.rows; r++)
	{
		for (int c = 0; c < redVegetation.cols; c++)
		{
			redVegetation.at<Vec3b>(r, c)[2] = max(veg.at<uchar>(r, c), grayscale.at<uchar>(r, c));
		}
	}

	imshow("Color Composite", colorComposite);
	imshow("Red Veggies", redVegetation);
	imshow("SWIR", swir);
	imwrite("ColorComposite.png", colorComposite);
	imwrite("RedVeg.png", redVegetation);
	imwrite("SWIR.png", swir);
	waitKey(0);
	
	return 0;
}
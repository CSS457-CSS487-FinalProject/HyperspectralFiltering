/*
Hyperspectral Filtering filters a hyperspectral image by a set
series of filter parameters that makeup the spectral resonance pattern
of objects of interest.

Authors:
Taran Christiansen - taranch@uw.edu
Christian Gebhart  - christian.gebhart@gmail.com
Anthony Pepe       - apepe@uw.edu

Date: November 2016
*/
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "SpecFilter.h"
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

void TaranTest()
{
	SpecImage newImg("EO1H0420342016268110PF_1T");

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
}

void SpecFilterTest()
{
	SpecImage hyperImage("EO1H0010492002110110KZ_1T");
	waitKey(0);
	SpecFilter filter;
	filter.LoadFromFile("douglas_fir.txt");
	Mat result = filter.filter(hyperImage);

	imshow("Original", hyperImage.getComposite(650, 580, 508));
	imwrite("Original.png", hyperImage.getComposite(650, 580, 508));
	imshow("Targets", result);
	imwrite("Fir Trees.png", result);
	waitKey(0);
}

int main(int argc, char* argv[])
{	
	//TaranTest();
	SpecFilterTest();
	return 0;
}
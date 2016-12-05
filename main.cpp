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

Mat img;
Mat thresh;
Mat opening;
Mat element;
int morph_elem = 0;
int morph_size = 0;
int morph_operator = 0;

void Morphology_Operations(int, void*)
{
	// Since MORPH_X : 2,3,4,5 and 6
	int operation = morph_operator + 2;

	element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));

	// Apply the specified morphology operation
	morphologyEx(thresh, opening, operation, element);
	imshow("Window", opening);
}

void WatershedTest()
{
	// Load the image
	//SpecImage hyperImage("EO1H0010492002110110KZ_1T");
	//img = hyperImage.getComposite(650, 580, 508);
	//img = hyperImage.getComposite(1954, 1629, 1074); // Short Wavelength InfraRed (SWIR)

	Mat gray;
	cvtColor(img, gray, CV_BGR2GRAY);

	threshold(gray, thresh, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);

	// noise removal
	namedWindow("Window", CV_WINDOW_AUTOSIZE);
	createTrackbar("Operator:\n 0: Opening - 1: Closing \n 2: Gradient - 3: Top Hat \n 4: Black Hat", "Window", &morph_operator, 4, Morphology_Operations);
	createTrackbar("Element:\n 0: Rect - 1: Cross - 2: Ellipse", "Window",
		&morph_elem, 2,
		Morphology_Operations);
	createTrackbar("Kernel size:\n 2n +1", "Window",
		&morph_size, 21,
		Morphology_Operations);
	Morphology_Operations(0, 0);
	waitKey(0);

	// sure background area
	Mat sure_bg;
	Mat kernel(3, 3, CV_8UC1);
	kernel = cv::Scalar(1);
	dilate(opening, sure_bg, kernel, Point(-1, -1), 3);

	// Finding sure foreground area
	Mat dist_transform;
	double maxVal;
	double minVal;
	minMaxLoc(opening, &minVal, &maxVal);
	threshold(opening, opening, 50, 255, CV_THRESH_BINARY);

	distanceTransform(opening, dist_transform, DIST_L2, 5);
	Mat sure_fg;
	minMaxLoc(dist_transform, &minVal, &maxVal);
	threshold(dist_transform, sure_fg, 4, 255, THRESH_TOZERO);

	// Finding unknown region
	sure_fg.convertTo(sure_fg, CV_8UC1);
	//cvtColor(sure_bg, sure_bg, CV_BGR2GRAY);
	Mat unknown;
	subtract(sure_bg, sure_fg, unknown);

	// Marker labelling
	Mat markers;
	connectedComponents(sure_fg, markers);

	// Add one to all labels so that sure background is not 0, but 1
	// Now, mark the region of unknown with zero
	for (int r = 0; r < markers.rows; r++)
	{
		for (int c = 0; c < markers.cols; c++)
		{
			if (unknown.at<uchar>(r, c) == 255)
			{
				markers.at<uchar>(r, c) = 0;
			}
			else
			{
				markers.at<uchar>(r, c) = markers.at<uchar>(r, c) + 1;
			}
		}
	}

	watershed(img, markers);

	for (int r = 0; r < img.rows; r++)
	{
		for (int c = 0; c < img.cols; c++)
		{
			if (markers.at<char>(r, c) == -1)
			{
				img.at<Vec3b>(r, c)[2] = 0;
				img.at<Vec3b>(r, c)[1] = 255;
				img.at<Vec3b>(r, c)[0] = 0;
			}
		}
	}

	imshow("Watershed", img);
	imwrite("Watershed.png", img);
	waitKey(0);
}

void TaranTest()
{
	SpecImage newImg("EO1H0010492002110110KZ_1T");

	Mat colorComposite = newImg.getComposite(641, 580, 509); // Hyperion reccomended color composite
	Mat swir = newImg.getComposite(1954, 1629, 1074); // Short Wavelength InfraRed (SWIR)

	Mat grayscale;
	Mat veg = newImg.getImage(855); // 16US1

	short arbitraryThreshold = 32768 / 64;
	veg.convertTo(veg, CV_8UC1, 255.0 / arbitraryThreshold);

	cvtColor(colorComposite, grayscale, CV_RGB2GRAY); // Convert to gray

	Mat redVegetationGray = grayscale.clone();
	Mat redVegetationColor = colorComposite.clone();
	cvtColor(redVegetationGray, redVegetationGray, CV_GRAY2BGR); // Return to 3 channels

	for (int r = 0; r < redVegetationColor.rows; r++)
	{
		for (int c = 0; c < redVegetationColor.cols; c++)
		{
			redVegetationColor.at<Vec3b>(r, c)[2] = max(veg.at<uchar>(r, c), colorComposite.at<Vec3b>(r, c)[2]);
			redVegetationGray.at<Vec3b>(r, c)[2] = max(veg.at<uchar>(r, c), grayscale.at<uchar>(r, c));
		}
	}

	imshow("Color Composite", colorComposite);
	imshow("Red Veggies Gray", redVegetationGray);
	imshow("Red Veggies Color", redVegetationColor);
	imshow("SWIR", swir);
	imwrite("ColorComposite.png", colorComposite);
	imwrite("RedVegColor.png", redVegetationColor);
	imwrite("RedVegGray.png", redVegetationGray);
	imwrite("SWIR.png", swir);
	waitKey(0);
}

void SpecFilterTest()
{
	SpecImage hyperImage("EO1H0010492002110110KZ_1T");
	waitKey(0);
	SpecFilter filter;
	filter.LoadFromFile("water.txt");
	Mat result = filter.filter(hyperImage);

	imshow("Original", hyperImage.getComposite(650, 580, 508));
	imwrite("Original.png", hyperImage.getComposite(650, 580, 508));
	imshow("Targets", result);
	imwrite("Fir Trees.png", result);
	waitKey(0);
}

void christianTest()
{
	SpecImage hyperImage("EO1H0460272003133110PW");
	waitKey(0);
	SpecFilter filterfir;
	filterfir.LoadFromFile("douglas_fir.txt");
	Mat resultTree = filterfir.filter(hyperImage);

	SpecFilter filterWater;
	filterWater.LoadFromFile("water.txt");
	Mat resultWater = filterWater.filter(hyperImage);

	Mat overlap = resultTree.clone();
	for (int row = 0; row < overlap.rows; row++)
	{
		for (int col = 0; col < overlap.cols; col++)
		{
			uchar wat = resultWater.at<uchar>(row, col);
			uchar trees = overlap.at<uchar>(row, col);
			if (overlap.at<uchar>(row, col) == 255 && resultWater.at<uchar>(row, col) == 255)
			{
				overlap.at<uchar>(row, col) = 255;
			}
			else
			{
				overlap.at<uchar>(row, col) = 0;
			}
		}
	}

	Mat waterAndTrees(overlap.rows, overlap.cols, CV_8UC3);
	for (int row = 0; row < waterAndTrees.rows; row++)
	{
		for (int col = 0; col < waterAndTrees.cols; col++)
		{
			if (resultWater.at<uchar>(row, col) == 255)
			{
				waterAndTrees.at<Vec3b>(row, col)[0] = 255;
			}
			else
			{
				waterAndTrees.at<Vec3b>(row, col)[0] = 0;
			}
			if(resultTree.at<uchar>(row, col) == 255)
			{
				waterAndTrees.at<Vec3b>(row, col)[2] = 255;
			}
			else
			{
				waterAndTrees.at<Vec3b>(row, col)[2] = 0;
			}
			waterAndTrees.at<Vec3b>(row, col)[1] = 0;
			Vec3b i = waterAndTrees.at<Vec3b>(row, col);
		}
	}

	
	imshow("Original", hyperImage.getComposite(650, 580, 508));
	imwrite("Original.png", hyperImage.getComposite(650, 580, 508));
	imshow("trees", resultTree);
	imwrite("Fir Trees.png", resultTree);
	imshow("water", resultWater);
	imwrite("water.png", resultWater);
	imshow("water and trees", waterAndTrees);
	imwrite("waterAndTrees.png", waterAndTrees);
	
	img = waterAndTrees;
	waitKey(0);
}

int main(int argc, char* argv[])
{
	//TaranTest();
	//SpecFilterTest();
	christianTest();
	WatershedTest();
}


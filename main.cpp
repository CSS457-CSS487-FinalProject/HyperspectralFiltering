/*
Hyperspectral Filtering filters a hyperspectral image by a set
series of filter parameters that makeup the spectral resonance pattern
of objects of interest.
Authors:
Taran Christiansen - taranch@uw.edu
Christian Gebhart  - christian.gebhart@gmail.com
Anthony Pepe       - apepe@uw.edu
Date: December 2016
*/
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "SpecFilter.h"
#include "SpecImage.h"

using namespace cv;
using namespace std;

//  type2str
//  This method takes a given OpenCV Mat type and returns the string 
//  representatiton of that type. Useful for debugging purposes (eg. determining
//  the bit depth of TIF images).
//  Pre-Conditions: type is the integer type of a Mat acquired by the code
//  matrix.type().
//  Post-Conditions: The string representation of the given type is returned  
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

//  Watershed
//  This method applies Watershed segmetation on the supplied image, returning
//  and image whose green channel is made up of the watershed lines. This image
//  is shown to the user at the end of the method (with a waitKey(0)).
//  Pre-Conditions: Img is a 8UC, wherein bright values indicate areas of
//  interest. Img is expected to be either grayscale (8UC1) or color (8UC3).
//  Post-Conditions: The original img is returned with watershed markings overlayed
//  in pure-green (0, 255, 0). Bright areas are outlined as areas of interest. The
//  returned Img is always 8UC3.
//  DISCLAIMER: This code is adapted from the example for watershed segmentnation
//  written in Python at the following link:
//  http:// docs.opencv.org/3.1.0/d3/db4/tutorial_py_watershed.html
Mat Watershed(Mat img)
{
	//  Create a binary threshold image
	Mat thresh;
	if (img.channels() == 1)
	{
		thresh = img;
		cvtColor(img, img, CV_GRAY2BGR);
	}
	else
	{
		cvtColor(img, thresh, CV_BGR2GRAY);
	}
	threshold(thresh, thresh, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);

	//  Noise removal
	int morph_size = 8;
	Mat element = getStructuringElement(2, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));

	//  Apply the specified morphology operation
	Mat opening;
	morphologyEx(thresh, opening, 2, element);

	//  Sure background area
	Mat sure_bg;
	Mat kernel(3, 3, CV_8UC1);
	kernel = cv::Scalar(1);
	dilate(opening, sure_bg, kernel, Point(-1, -1), 3);

	//  Finding sure foreground area
	Mat dist_transform;
	double maxVal;
	double minVal;
	minMaxLoc(opening, &minVal, &maxVal);
	threshold(opening, opening, 50, 255, CV_THRESH_BINARY);

	distanceTransform(opening, dist_transform, DIST_L2, 5);
	Mat sure_fg;
	minMaxLoc(dist_transform, &minVal, &maxVal);
	threshold(dist_transform, sure_fg, 4, 255, THRESH_TOZERO);

	//  Finding unknown region
	sure_fg.convertTo(sure_fg, CV_8UC1);
	Mat unknown;
	subtract(sure_bg, sure_fg, unknown);

	//  Marker labelling
	Mat markers;
	connectedComponents(sure_fg, markers);

	//  Add one to all labels so that sure background is not 0, but 1
	//  Also mark the region of unknown with zero
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

	//  Apply watershed method
	watershed(img, markers);

	//  Draw markekrs in pure-green on image
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

	//  Show generaed watershed image with markers
	imshow("Watershed", img);
	imwrite("Watershed.png", img);
	waitKey(0);

	return img;
}

//  FindVegetation
//  This method takes a given SpecImage and displays the images listed below: 
//  		-Original Color composite
//  		-Short-Wave-Infared (SWIR) "hypercolor" image
//  		-Vegetation health map (red on grayscale)
//  		-Vegetation health map composite (red on color)
//  Pre-Conditions: Supplied hyperImage exists and is non-empty 
//  Post-Conditions: Returns the gray-red map of the vegetation health, where areas
//  of medium to high vegetation health are dispalyed, and areas of low vegetation
//  health (those areas unlikely to have vegetation) are displayed as gray.
Mat FindVegetation(SpecImage hyperImage)
{
	Mat colorComposite = hyperImage.getComposite(641, 580, 509); //  Hyperion reccomended color composite
	Mat swir = hyperImage.getComposite(1954, 1629, 1074); //  Short Wavelength InfraRed (SWIR)
	
	Mat grayscale;
	Mat veg = hyperImage.getImage(855); //  16US1

	short arbitraryThreshold = 32768 / 64;
	veg.convertTo(veg, CV_8UC1, 255.0 / arbitraryThreshold);

	cvtColor(colorComposite, grayscale, CV_RGB2GRAY); //  Convert to gray

	Mat redVegetationGray = grayscale.clone();
	Mat redVegetationColor = colorComposite.clone();
	cvtColor(redVegetationGray, redVegetationGray, CV_GRAY2BGR); //  Return to 3 channels

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

	return redVegetationGray;
}

//  SpecFilterTest
//  This method takes a given SpecImage and a filter name and displays it's 
//  resulting filter map
//  Pre-Conditions: Supplied hyperImage exists and is non-empty 
//  Post-Conditions: Returns the grayscale filtered map created by filtering the
//  supplied hyperImage with the filter "[filterName].txt".
Mat SpecFilterTest(SpecImage hyperImage, string filterName)
{
	SpecFilter filter;
	filter.LoadFromFile(filterName + ".txt");
	Mat result = filter.filter(hyperImage);

	imshow("Original", hyperImage.getComposite(650, 580, 508));
	imwrite("Original.png", hyperImage.getComposite(650, 580, 508));
	imshow("Targets", result);
	imwrite("Fir Trees.png", result);
	waitKey(0);

	return result;
}

//  TreesWaterFilter
//  This method takes a given SpecImage and displays the images listed below: 
//  		-Original Color composite
//  		-Trees (douglas_fir) filter image
//  		-Water filter image
//  		-Trees (red) and Water (blue) composite image
//  Pre-Conditions: Supplied hyperImage exists and is non-empty 
//  Post-Conditions: Returns the red/blue filtered map created by comining the 
//  resulting filtermaps of hyperImage with the filter "water.txt" and the filter
//  "douglas_fir.txt". Areas in the image that are red are those areas tat have
//  trees, whereas blue areas are those with water.
Mat TreesWaterFilter(SpecImage hyperImage)
{
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
	waitKey(0);

	return waterAndTrees;
}

//  main
//  Run the specified methods, given the supplied SpecImage
//  Pre-Conditions: The folder given as a paramater for newSpecImg exists, and 
//  contains the correct images with the correct filenames.
//  Post-Conditions: Runs the uncommented methods, each of which is detailed
//  above
int main(int argc, char* argv[])
{
	SpecImage newSpecImg("EO1H0010492002110110KZ_1T");

	Mat img;
	//  img = FindVegetation(newSpecImg);
	//  img = SpecFilterTest(newSpecImg, "douglas_fir");
	img = TreesWaterFilter(newSpecImg);
	Mat watershed = Watershed(img);
}


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

	Mat blue = newImg.getImage(532);
	imshow("Blue", blue);
	waitKey(0);

	/* OLD CODE */
	return 0; // REM out to run the following code

	// Load three (fairly arbitrary) images 
	cv::Mat R = cv::imread("EO1H0420342016268110PF_B035_L1T.TIF", -1); // 035 = 701 nm
	cv::Mat G = cv::imread("EO1H0420342016268110PF_B018_L1T.TIF", -1); // 018 = 528 nm 
	cv::Mat B = cv::imread("EO1H0420342016268110PF_B011_L1T.TIF", -1); // 011 = 457 nm

	// Print TIF bitdepth for debugging
	std::cout << "Mat G type: " << type2str(G.type()) << std::endl;

	// cv::merge takes a vector of Mat
	std::vector<cv::Mat> array_to_merge;

	// Use Mat.convertTo to change bit depth of our images to 8 bit chars
	//  from 16 bit chars, with normalization (otherwise the image is way
	//	too dim)
	Mat r2;
	Mat g2;
	Mat b2;

	int Max = 256 * 16; // The max value here is a guess-timation
	int Min = 0;

	B.convertTo(b2, CV_8U, 255.0 / (Max - Min), -255.0*Min / (Max - Min));
	G.convertTo(g2, CV_8U, 255.0 / (Max - Min), -255.0*Min / (Max - Min));
	R.convertTo(r2, CV_8U, 255.0 / (Max - Min), -255.0*Min / (Max - Min));

	array_to_merge.push_back(b2);
	array_to_merge.push_back(g2);
	array_to_merge.push_back(r2);

	cv::Mat_<Vec3b> color; // Our resulting 24-bit 3 channel (RGB) image

	cv::merge(array_to_merge, color);

	// Write / Show the result
	imwrite("merged.png", color);
	cout << "Color type: " << type2str(color.type()) << endl;
	imshow("Original Merged", color);
	waitKey(0);
	
	// Create a new image where we can change the values
	Mat_<Vec3b> new_Color = color.clone();

	for (int r = 1; r < color.rows - 1; r++)
	{
		for (int c = 1; c < color.cols - 1; c++)
		{
			// Make the green channel 30% stronger
			new_Color(r, c)[1] = saturate_cast<uchar>(1.3 * color(r, c)[1]);
		}
	}
	
	// Show this image (no write)
	imshow("Modified Merged", new_Color);

	/* CLAHE modification */
	// Try out CLAHE image resolution: 
	//  A method to make an image brighter by (I think:) converting it to a 
	//  different color space and normalizing the "lightness" in that entire
	//	new color space. It's supposed to improve the look of an image. Not
	//  sure on it, so I left it in.
	cv::Mat lab_image;
	cv::cvtColor(new_Color, lab_image, CV_BGR2Lab);

	// Extract the L channel
	std::vector<cv::Mat> lab_planes(3);
	cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

	// apply the CLAHE algorithm to the L channel
	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(10);
	cv::Mat dst;
	clahe->apply(lab_planes[0], dst);

	// Merge the the color planes back into an Lab image
	dst.copyTo(lab_planes[0]);
	cv::merge(lab_planes, lab_image);

	// convert back to RGB
	cv::Mat image_clahe;
	cv::cvtColor(lab_image, image_clahe, CV_Lab2BGR);

	// display the results  (you might also want to see lab_planes[0] before and after).
	cv::imshow("CLAHE Merged", image_clahe);
	cv::waitKey();
	imwrite("CLAHE.png", image_clahe);
	return 0;
}
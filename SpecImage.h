/*
SpecImage collects the hundreds of separate images
the makeup a single hyperspectral image so the images
are easier to manage and fetch.
*/

#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <string>

using namespace cv;
using namespace std;

class SpecImage
{
	public:
		// Loads a series of images. See LoadFromFile
		SpecImage(string fileName);

		// Load image set based on the fileName. Expects a Hyperion satellite dataset, with images
		//	    B001 through B242. All files are expected in TIF format. 
		// Ex1: "EO1H0460272013279110KF" loads files "EO1H0460272013279110KF_B001_L1GST"
		//   through "EO1H0460272013279110KF_B242_L1GST"
		// Ex2: "EO1H0420342016268110PF_1T" loads files "EO1H0420342016268110PF_B001_L1T"
		//   through "EO1H0420342016268110PF_B242_L1T"
		void LoadFromFile(string fileName);

		// Returns the image of a given wavelength
		Mat getImage(int wavelength) const;

		// Returns the height of the hyperspectral image (height of a single image).
		int getRows() const;

		// Returns the width of the hyperspectral image (width of a single image).
		int getCols() const;

		// Returns the number of wavelengths present in the hyperspectral image.
		int getDepth() const;

		// Returns the RGB estimation of this image.
		// This is achieved by supercomposing the wavelengths 380nm through 730nm.
		Mat getRGB();

		/*
		Creates a composite image by stacking three specific wavelength images ontop of each other
		into a single composite image where the first, second, and third wavelength-images are
		turned into the red, green, and blue channels of the compositie image respectively.
		*/
		Mat getComposite(int redWavelength, int blueWavelength, int greenWavelength);

		/*
		STATIC method to make a composite image given three grayscale images where
		the first, second, and third images make up the red, blue, and green channels
		of the composite image respectively.

		Returns a composite image given three grayscale images (such as those given via
		getImage()) Expects the same dimensions, and converts them to CV_8U
		*/
		Mat makeComposite(Mat redImage, Mat blueImage, Mat greenImage);
	private:
		struct imgData
		{
			int wavelength;
			Mat img;
		};

		vector<imgData> specImg;
		static vector<int> hyperionWavelengthTable;

		static void initilizeWavelengthTable();
};
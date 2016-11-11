#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <string>

#include "SpecFilter.h"

using namespace cv;
using namespace std;

class SpecImage
{
	public:
		SpecImage(string fileName); // Uses the below LoadFromFile

		// Load image set from fileName. Expects a Hyperion dataset, with images B001 through
		//		B242. All files are expected in TIF format. 
		//	 Ex1: "EO1H0460272013279110KF" loads files "EO1H0460272013279110KF_B001_L1GST"
		//		through "EO1H0460272013279110KF_B242_L1GST"
		//	 Ex2: "EO1H0420342016268110PF_1T" loads files "EO1H0420342016268110PF_B001_L1T"
		//		through "EO1H0420342016268110PF_B242_L1T"
		void LoadFromFile(string fileName);

		// Return the image of a given wavelength
		Mat getImage(int wavelength) const;

		// Return the RGB estimation of this image (using 380nm through 730nm)
		Mat getRGB();

		Mat getComposite(int redWavelength, int blueWavelength, int greenWavelength);
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
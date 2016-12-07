/*
SpecImage
Defines a Spectral Image, which contains hundreds of separate images taken of the
 same location at varying wavelengths over a large range. This object is designed
 to work with a dataset provided by the EO_1 Hyperion satellite. The purpose of
 this object is to make it easier to manage and fetch data in the provided image,
 to ease filtering of the data.
*/

#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <string>
#include <iostream>

using namespace cv;
using namespace std;

class SpecImage
{
	public:
		// SpecImage
		// Creates a new SpecImage object, loads the hyperionWavelengthTable, and loads 
		//  spectral images based on the image's root file name. See LoadFromFile for more 
		//  information on loading spectral images.
		// Pre-Condition: Filename refers to a folder of Hyperion hyperspectral satellite
		//  images that have not been renamed. These images are expected to be in the 
		//  GeoTIF format.
		// Post-Condition: Images from the specified folder are loaded into this SpecImage
		//  object, and can be accessed by SpecImage methods.
		SpecImage(string fileName);

		// LoadFromFile
		// Creates a new Spectral Image based on the image's root file name. This is done 
		//  by dynamically generating file names because Hyperion's list of spectral 
		//  images follow a set pattern.
		// Pre-Condition: Filename refers to a folder of Hyperion hyperspectral satellite 
		//  images that have not been renamed. These images are expected to be in the 
		//  GeoTIF format, with 242 images named B001 through B242 (see examples).
		// Post-Condition: Images from the specified folder are loaded into this SpecImage
		//  object, and can be accessed by SpecImage methods.
		// Ex1: "EO1H0460272013279110KF" loads files "EO1H0460272013279110KF_B001_L1GST"
		//   through "EO1H0460272013279110KF_B242_L1GST"
		// Ex2: "EO1H0420342016268110PF_1T" loads files "EO1H0420342016268110PF_B001_L1T"
		//   through "EO1H0420342016268110PF_B242_L1T"
		void LoadFromFile(string fileName);

		// getImage
		// Fetches a single spectral image, which is specified by its wavelength.
		// Pre-Condition: None
		// Post-Condition: Returns the nearest wavelength image (OpenCV Mat). Since the 
		//  images are specified by a bounding set of wavelengths, the nearest wavelength
		//  is returned because an image for that exact wavelength may not exist. If the 
		//  image is out of range, an empty Mat is returned.
		Mat getImage(int wavelength) const;

		// getRows
		// Returns the height of the hyperspectral image (height of a single image).
		// Pre-Condition: None
		// Post-Condition: Returns an integer representing the height of the SpecImage.
		int getRows() const;

		// getCols
		// Returns the width of the hyperspectral image (width of a single image).
		// Pre-Condition: None
		// Post-Condition: Returns an integer representing the width of the SpecImage.
		int getCols() const;

		// getDepth
		// Returns the number of wavelengths present in the hyperspectral image.
		// Pre-Condition: None
		// Post-Condition: Returns an integer representing the total depth (eg. number of
		//  images or wavelengths) in this SpecImage.
		int getDepth() const;

		// DEPRECATED
		// getRGB
		// Returns an RGB image estimation of this hyperspectral image.
		// Pre-Condition: The SpecImage this is called on exists, and is non-empty.
		// Post-Condition: Returns the color representation of the SpecImage as determined 
		//  by the 1931 CIE Color Data and 36 separate wavelength images.
		// NOTE: This method was deprecated as we found that the results were unideal (in
		//  terms of color accuracy. Moreover, we found this method of manipulating data 
		//  across multiple images to be too time consuming for the user, as a simple 
		//  composite of the correct wavelengths can create a color representation 
		//  equivlent to that taken by a standard camera.
		Mat getRGB();

		// getComposite
		// Creates a composite image by stacking three specific wavelength images ontop of 
		//  each other into a single composite image where the first, second, and third 
		//  wavelength-images are turned into the red, green, and blue channels of the 
		//  compositie image respectively.
		// Pre-Condition: The SpecImage this is called on exists, and is non-empty. The 
		//  wavelengths supplied are to be in the range of 356nm to 2600nm.
		// Post-Condition: Returns an 8UC3 image created as a result of using three 
		//  grayscale images acquired from the provided wavelengths. The variables (eg 
		//  redWaveLength) coorespond to the color channel they will fill (eg R). 
		Mat getComposite(int redWavelength, int blueWavelength, int greenWavelength);

		// makeComposite
		// STATIC method to make a composite image given three grayscale images where
		//  the first, second, and third images make up the red, blue, and green channels
		//  of the composite image respectively.
		// Pre-Condition: The Mat images supplied exist, and are non-empty. These images 
		//  are expected to be grayscale images. 
		// Post-Condition: Returns an 8UC3 image created as a result of using thre three 
		//  grayscale images provided. The variables (eg redImage) coorespond to the color 
		//  channel they will fill (eg R).
		// NOTE: The returned pointer is created in this function and must be deleted by 
		//  the calling function.
		Mat makeComposite(Mat redImage, Mat blueImage, Mat greenImage);
	private:
		struct imgData
		{
			int wavelength;
			Mat img;
		};

		vector<imgData> specImg;
		static vector<int> hyperionWavelengthTable;

		// initilizeWavelengthTable
		// Private method to load the Hyperion Wavelength table
		// Pre-conditions: None
		// Post-conditions: The SpecImage wavelength table is loaded (run at creation)
		// Developer Notes:  Hyperion has an odd part of it's wavelength table where bands 
		//  71 through 91 overlap with 50 through 70. This is taken into account here. 
		//  This could be improved by importing the actual table, but this estimation is 
		//  decently accurate.
		static void initilizeWavelengthTable();
};
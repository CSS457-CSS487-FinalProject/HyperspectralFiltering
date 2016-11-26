/*
SpecFilter represents a resonance pattern of a physical object.
Spectral images can be filtered to determine if (and where) the target objects
are present, in a hyperspectral image.

The filter stores a map of reflectance values where each wavelength has some normalized
value of expected reflectance at that wavelength. The Sum of Absolute Differences (SAD)
technique is used to compare the hyperspectral image against the filter.

@author Anthony Pepe
*/
#pragma once
#include <opencv2/core/core.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>

#include "SpecImage.h"

using namespace cv;
using namespace std;

class SpecFilter
{
	public:
		/*
		  Creates a new filter with no values for any wavelength. Users
		  can set wavelength-reflectance values directly via SetIntensity(),
		  or can upload a USGS reflectance file via LoadFromFile().
		*/
		SpecFilter();

		/*
		  Returns the reflectance intensity of the filter at the specified wavelength.
		  @param wavelength Wavelength in nanometers. (1,000 nanometers = 1 micrometer)
		  @return Reflectance intensity of the filter at that wavelength.
		*/
		double GetIntensityNano(const int& wavelength) const;

		/*
		  Returns the reflectance intensity of the filter at the specified wavelength.
		  @param wavelength Wavelength in micrometers.
		  @return Reflectance intensity of the filter at that wavelength.
		*/
		double GetIntensityMicro(const double& wavelength) const;

		/*
		  Sets the reflectance intensity of the filter at the specified wavelength.
		  The intensity must be between 0 and 1.
		  @param wavelength Wavelength in nanometers. (1,000 nanometers = 1 micrometer)
		  @param intensity Reflectance intensity of the filter. Must be between 0 and 1.
		*/
		void SetIntensityNano(const int& wavelength, const double& intensity);

		/*
		  Sets the reflectance intensity of the filter at the specified wavelength.
		  The intensity must be between 0 and 1.
		  @param wavelength Wavelength in micrometers.
		  @param intensity Reflectance intensity of the filter. Must be between 0 and 1.
		*/
		void SetIntensityMicro(const double& wavelength, const double& intensity);

		/*
		  Loads filter data from a USGS formatted Reflectance Pattern file. Previous filter
		  data may be overwritten during this process.
		  @param filename Relative file location to load.
		  @return True if the file was read successfully, false otherwise.
		  @see http://speclab.cr.usgs.gov/spectral.lib06/ds231/datatable.html
		*/
		bool LoadFromFile(string fileName);

		/*
		  Finds pixles in a target image that have similar reflectance values to this filter.
		  Returns a greyscale image where black and dark pixels are "poor matches" and bright or
		  white pixels are "strong matches."
		  @param image Hyperspectral image to run this filter against.
		  @return Greyscale image (type CV_8UC1) where bright/white pixels correspond to positive
		  results from this filter.
		*/
		Mat filter(const SpecImage& image) const;

	private:
		map<double, double> filterData;
};
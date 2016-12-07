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
		
		//  SpecFilter
		//  Creates a new filter with no values for any wavelength. Users
		//  can set wavelength-reflectance values directly via SetIntensity(),
		//  or can upload a USGS reflectance file via LoadFromFile().
		SpecFilter();

		//  GetIntensityNano
		//  Returns the reflectance intensity of the filter at the specified wavelength.
		//  Pre-Conditions: The wavelength provided must be in nanometers
		//  Post-Conditions: The Reflectance intensity of the filter at that wavelength 
		//  is returned.
		double GetIntensityNano(const int& wavelength) const;

		//  GetIntensityMicro
		//  Returns the reflectance intensity of the filter at the specified wavelength.
		//  Pre-Conditions: The wavelength provided must be in micrometers
		//  Post-Conditions: The Reflectance intensity of the filter at that wavelength 
		//  is returned.
		double GetIntensityMicro(const double& wavelength) const;

		//  SetIntensityNano
		//  Sets the reflectance intensity of the filter at the specified wavelength.
		//  The intensity must be between 0 and 1.
		//  Pre-Conditions: wavelength Wavelength in nanometers. (1,000 nanometers = 1 
		//  micrometer)
		//  Post-Conditions: Reflectance intensity of the filter is returned. The 
		//  value must be between 0 and 1.
		void SetIntensityNano(const int& wavelength, const double& intensity);

		//  SetIntensityMicro
		//  Sets the reflectance intensity of the filter at the specified wavelength.
		//  The intensity must be between 0 and 1.
		//  Pre-Conditions: wavelength Wavelength in micrometers.
		//  Post-Conditions: Reflectance intensity of the filter is returned. The 
		//  value must be between 0 and 1.
		void SetIntensityMicro(const double& wavelength, const double& intensity);
	
		//  LoadFromFile
		//  Loads filter data from a USGS formatted Reflectance Pattern file. Previous filter
		//  data may be overwritten during this process.
		//  Pre-Conditions: The filename Relative file location to load must be provided, 
		//  and the file must exist and be properly formatted (reflectance values start at 
		//  line 16, etc.).
		//  Post-Conditions: return True if the file was read successfully, false otherwise.
		//  On success the reflectance values will be read in an stored for filtering.
		//  see http://speclab.cr.usgs.gov/spectral.lib06/ds231/datatable.html
		bool LoadFromFile(string fileName);

		//  filter
		//  Finds pixles in a target image that have similar reflectance values to this filter.
		//  Returns a greyscale image where black and dark pixels are "poor matches" and bright or
		//  white pixels are "strong matches."
		//  Pre-Conditions: A image Hyperspectral image to run this filter against 
		//  must be passed in
		//  Post-Conditions: A greyscale image (type CV_8UC1) where bright/white 
		//  pixels indicate a likely match to the object type being searched for.
		Mat filter(const SpecImage& hyperImage) const;

	private:
		map<double, double> filterData;
};
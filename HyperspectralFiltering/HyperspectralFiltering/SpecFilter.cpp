/*
	Implements SpecFilter.h
	@author Anthony Pepe
*/
#include "SpecFilter.h"

/*
Creates a new filter with no values for any wavelength. Users
can set wavelength-reflectance values directly via SetIntensity(),
or can upload a USGS reflectance file via LoadFromFile().
*/
SpecFilter::SpecFilter()
{
	// Nothing to construct
}

/*
Returns the reflectance intensity of the filter at the specified wavelength.
@param wavelength Wavelength in nanometers. (1,000 nanometers = 1 micrometer)
@return Reflectance intensity of the filter at that wavelength.
*/
double SpecFilter::GetIntensityNano(const int& wavelength) const
{
	double wavelengthMicro = wavelength / 1000.0;
	return GetIntensityMicro(wavelengthMicro);
}

/*
Returns the reflectance intensity of the filter at the specified wavelength.
@param wavelength Wavelength in micrometers.
@return Reflectance intensity of the filter at that wavelength.
*/
double SpecFilter::GetIntensityMicro(const double& wavelength) const
{
	map<double, double>::const_iterator iter = filterData.find(wavelength);
	if (iter == filterData.end)
	{
		return 0;
	}
	return iter->second;
}

/*
Sets the reflectance intensity of the filter at the specified wavelength.
The intensity must be between 0 and 1.
@param wavelength Wavelength in nanometers. (1,000 nanometers = 1 micrometer)
@param intensity Reflectance intensity of the filter. Must be between 0 and 1.
*/
void SpecFilter::SetIntensityNano(const int& wavelength, const double& intensity)
{
	double wavelengthMicro = wavelength / 1000.0;
	return SetIntensityMicro(wavelengthMicro, intensity);
}

/*
Sets the reflectance intensity of the filter at the specified wavelength.
The intensity must be between 0 and 1.
@param wavelength Wavelength in micrometers.
@param intensity Reflectance intensity of the filter. Must be between 0 and 1.
*/
void SpecFilter::SetIntensityMicro(const double& wavelength, const double& intensity)
{
	filterData[wavelength] = intensity;
}

/*
Loads filter data from a USGS formatted Reflectance Pattern file. Previous filter
data may be overwritten during this process.
@param filename Relative file location to load.
@return True if the file was read successfully, false otherwise.
@see http://speclab.cr.usgs.gov/spectral.lib06/ds231/datatable.html
*/
bool SpecFilter::LoadFromFile(string fileName)
{
	ifstream inputFile(fileName);
	if (!inputFile.is_open())
	{
		cerr << "Error - Could not find file \"" << fileName << "\"." << endl;
		return false;
	}

	string line;
	int lineCount = 0;
	while (!inputFile.eof())
	{
		getline(inputFile, line);
		if (lineCount < 17) // Data starts at line 17
		{
			++lineCount;
			continue;
		}
		if (line.find("-1.23e34") == string::npos) // Indicates data does not exist.
		{
			continue;
		}

		stringstream lineIn(line);
		double wavelength, reflectance;
		lineIn >> wavelength;
		lineIn >> reflectance;

		filterData[wavelength] = reflectance;
	}

	return true;
}

/*
Finds pixles in a target image that have similar reflectance values to this filter.
Returns a greyscale image where black and dark pixels are "poor matches" and bright or
white pixels are "strong matches."
@param image Hyperspectral image to run this filter against.
@return Greyscale image (type CV_8UC1) where bright/white pixels correspond to
positive results from this filter.
*/
Mat SpecFilter::filter(const SpecImage& hyperImage) const
{
	map<double, double>::const_iterator i = filterData.begin();

	int rows = hyperImage.getRows();
	int cols = hyperImage.getCols();
	int dims[] = { rows, cols };
	Mat histogram(2, dims, CV_64F, Scalar::all(0));

	while (i != filterData.end)
	{
		int wavelength = static_cast<int>(i->first * 1000); // convert back to nanometers
		double searchReflectance = i->second;
		Mat image = hyperImage.getImage(wavelength);
		image.convertTo(image, CV_8UC1);

		for (int col = 0; col < cols; ++col)
		{
			for (int row = 0; row < rows; ++row)
			{
				double imageReflectance = image.at<uchar>(row, col) / 255.0;
				histogram.at<double>(row, col) += abs(searchReflectance - imageReflectance);
			}
		}
	}

	double min = DBL_MAX, max = DBL_MIN;
	for (int col = 0; col < cols; ++col)
	{
		for (int row = 0; row < rows; ++row)
		{
			double val = histogram.at<double>(row, col);
			if (val < min)
			{
				min = val;
			}
			if (val > max)
			{
				max = val;
			}
		}
	}

	// There are 224 possible channels (wavelengths). If we set a max allowable difference of 2.5% intensity
	// per channel, that's 560 total possible difference.
	const double MATCH_MAX = 560;
	Mat resultImage(2, dims, CV_8UC1, Scalar::all(0));
	for (int col = 0; col < cols; ++col)
	{
		for (int row = 0; row < rows; ++row)
		{
			double val = histogram.at<double>(row, col);
			if (val >= MATCH_MAX)
			{
				continue;
			}
			uchar pixelValue = 255 - static_cast<uchar>(255 * (val / MATCH_MAX));
			resultImage.at<uchar>(row, col) = pixelValue;
		}
	}

	return resultImage;
}
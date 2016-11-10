#include "SpecImage.h"

// TEMPORARY
#include <iostream>

vector<int> SpecImage::hyperionWavelengthTable;

SpecImage::SpecImage(string fileName)
{
	if (hyperionWavelengthTable.capacity() != 242)
	{
		cout << "Creating wavelength table.." << endl;
		initilizeWavelengthTable();
		cout << "Wavelength table created" << endl;
	}
	cout << "Loading image data.." << endl;
	LoadFromFile(fileName);
	cout << "Image data loaded.." << endl;
}

void SpecImage::LoadFromFile(string fileName)
{
	bool L1T = false; // False: L1GST filetype
	if (fileName.length() > 3 && fileName.substr(fileName.length() - 3, fileName.length()) == "_1T")
	{
		L1T = true;
		fileName += "/" + fileName.substr(0, fileName.length() - 3);
		//cout << "FileName: " << fileName << endl;
	}
	else
	{
		fileName += "/" + fileName;
	}

	for (int i = 1; i <= 242; i++)
	{
		Mat img;
		if (L1T)
		{
			if (i / 100 > 0)
			{
				img = imread(fileName + "_B" + to_string(i) + "_L1T.TIF", -1);
			}
			else if (i / 10 > 0)
			{
				img = imread(fileName + "_B0" + to_string(i) + "_L1T.TIF", -1);
			}
			else
			{
				img = imread(fileName + "_B00" + to_string(i) + "_L1T.TIF", -1);
				//cout << "ImgName: " << (fileName + "_B" + to_string(i) + "_L1T.TIF") << endl;
			}
		}
		else
		{
			if (i / 100 > 0)
			{
				img = imread(fileName + "_B" + to_string(i) + ".TIF", -1);
			}
			else if (i / 10 > 0)
			{
				img = imread(fileName + "_B0" + to_string(i) + ".TIF", -1);
			}
			else
			{
				img = imread(fileName + "_B00" + to_string(i) + ".TIF", -1);
			}
		}

		specImg.push_back({hyperionWavelengthTable[i-1],img});
	}
}

// Given a wavelength to get from the spectral image, return the nearest saved
//  wavelength matrix 
Mat SpecImage::getImage(int wavelength) const
{
	// Return something here on invalid params (-1, less than 355ish, etc)

	// Estimate the closest wavelength image..
	int index;
	if (wavelength <= 844)
	{
		index = static_cast<int>(round((wavelength - 355.59f)/10.175f));
	}
	else if (wavelength >= 1063)
	{
		index = static_cast<int>(round((wavelength - 851.92f) / 10.09f));
	}
	else
	{
		// Crazy overlap area
		int index1 = static_cast<int>(round((wavelength - 355.59f) / 10.175f));
		int index2 = static_cast<int>(round((wavelength - 851.92f) / 10.09f));

		// Check which result gives us a closer wavelength, use that
		if (abs(wavelength - hyperionWavelengthTable[index1]) < abs(wavelength - hyperionWavelengthTable[index2]))
		{
			index = index1;
		}
		else
		{
			index = index2;
		}
	}

	cout << "Wavelength requested:\t" << wavelength << "\t Wavelength served:\t" << specImg[index].wavelength << endl;

	return specImg[index].img;
}

// Return the RGB estimation of this hyperspectral image
Mat getRGB()
{
	Mat newMat;



	return newMat;
}

// Load the Hyperion Wavelength table
// Hyperion has an odd part of it's wavelength table where
//  bands 71 through 91 overlap with 50 through 70. This is
//  taken into account here.
// This could be improved by importing the actual table, but
//  this estimation is decently accurate.
void SpecImage::initilizeWavelengthTable()
{
	hyperionWavelengthTable.reserve(242); // Reserve to reduce resizing
	for (int i = 0; i < 242; i++)
	{
		if (i < 69)
		{
			hyperionWavelengthTable.push_back(static_cast<int>(355.59f + i*10.175f));
			continue;
		}
		else
		{
			hyperionWavelengthTable.push_back(static_cast<int>(851.92f + (i-70)*10.09f));
		}
	}
}
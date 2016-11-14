/*
Defines a Spectral Image, which contains hundreds of separate images
take of the same location at very different wavelengths.
*/
#include "SpecImage.h"

// Debuging
#include <iostream>

vector<int> SpecImage::hyperionWavelengthTable;

/*
Creates a new Spectral Image based on the image's root file name.
Initializes the static table of hyperion satellite wavelengths, if
it has not been initialized yet.
See LoadFromFile
*/
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
	cout << "Image data loaded" << endl;
}

/*
Creates a new Spectral Image based on the image's root file name.
This is done by dynamically generating file names since Hyperion's
list of spectral images follow a set pattern.
*/
void SpecImage::LoadFromFile(string fileName)
{
	// Determine if the file types are Hyperion's L1T file type or not.
	bool L1T = false;
	if (fileName.length() > 3 && fileName.substr(fileName.length() - 3, fileName.length()) == "_1T")
	{
		L1T = true;
		fileName += "/" + fileName.substr(0, fileName.length() - 3);
	}
	else
	{
		fileName += "/" + fileName;
	}

	// For each spectral band image in the hypersepectral image...
	//     Generate the file name,
	//     Read the file into memory and store it in the vector
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
	} // end for-loop of spectral images.
}

/*
Fetches a single spectral image, which is specified by its wavelength.
Since the images are specified by a bounding set of wavelengths, the nearest
wavelength is returned since an image for that exact wavelength may not exist.
Returns the nearest wavelength image (OpenCV Mat).
*/
Mat SpecImage::getImage(int wavelength) const
{
	if (wavelength < 350 || wavelength > 2600)
	{
		//return NULL;
	}

	// Estimate the closest wavelength image
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

	return specImg[index].img;
}


/*
Returns an RGB image estimation of this hyperspectral image.
*/
Mat SpecImage::getRGB()
{
	Mat_<Vec3b> rgb(specImg[0].img.rows, specImg[0].img.cols);

	// End-Image Gamma 
	float gamma = 1.0f;

	// Used for normilization (based on Hyperion Satellite)
	const ushort maxShort = 32768 / 4;

	// 1931 CIE Data
	const float colorMatchingFunc[72][4] = {
		{ 380, 1.368000056e-03, 3.899999865e-05, 6.450001150e-03 },
		{ 385, 2.236000029e-03, 6.399999984e-05, 1.054999046e-02 },
		{ 390, 4.242999945e-03, 1.199999970e-04, 2.005000971e-02 },
		{ 395, 7.650000043e-03, 2.169999934e-04, 3.621000051e-02 },
		{ 400, 1.431000046e-02, 3.959999885e-04, 6.785000861e-02 },
		{ 405, 2.318999916e-02, 6.399999838e-04, 1.102000028e-01 },
		{ 410, 4.351000115e-02, 1.210000017e-03, 2.073999941e-01 },
		{ 415, 7.762999833e-02, 2.180000069e-03, 3.713000119e-01 },
		{ 420, 1.343799978e-01, 4.000000190e-03, 6.456000209e-01 },
		{ 425, 2.147700042e-01, 7.300000172e-03, 1.039050102e+00 },
		{ 430, 2.838999927e-01, 1.159999985e-02, 1.385599971e+00 },
		{ 435, 3.285000026e-01, 1.683999971e-02, 1.622959971e+00 },
		{ 440, 3.482800126e-01, 2.300000004e-02, 1.747059941e+00 },
		{ 445, 3.480600119e-01, 2.979999967e-02, 1.782600045e+00 },
		{ 450, 3.361999989e-01, 3.799999878e-02, 1.772109985e+00 },
		{ 455, 3.186999857e-01, 4.800000042e-02, 1.744099975e+00 },
		{ 460, 2.908000052e-01, 5.999999866e-02, 1.669199944e+00 },
		{ 465, 2.511000037e-01, 7.389999926e-02, 1.528100014e+00 },
		{ 470, 1.953600049e-01, 9.098000079e-02, 1.287639976e+00 },
		{ 475, 1.421000063e-01, 1.125999987e-01, 1.041900039e+00 },
		{ 480, 9.564000368e-02, 1.390199959e-01, 8.129500747e-01 },
		{ 485, 5.795000866e-02, 1.693000048e-01, 6.161999702e-01 },
		{ 490, 3.201000020e-02, 2.080200016e-01, 4.651800096e-01 },
		{ 495, 1.470000017e-02, 2.585999966e-01, 3.533000052e-01 },
		{ 500, 4.900000058e-03, 3.230000138e-01, 2.720000148e-01 },
		{ 505, 2.400000114e-03, 4.072999954e-01, 2.123000026e-01 },
		{ 510, 9.300000034e-03, 5.030000210e-01, 1.581999958e-01 },
		{ 515, 2.910000086e-02, 6.082000136e-01, 1.116999984e-01 },
		{ 520, 6.327000260e-02, 7.099999785e-01, 7.824999094e-02 },
		{ 525, 1.096000001e-01, 7.932000160e-01, 5.725001171e-02 },
		{ 530, 1.655000001e-01, 8.619999886e-01, 4.216000065e-02 },
		{ 535, 2.257498950e-01, 9.148501158e-01, 2.983999997e-02 },
		{ 540, 2.903999984e-01, 9.539999962e-01, 2.030000091e-02 },
		{ 545, 3.596999943e-01, 9.803000093e-01, 1.339999959e-02 },
		{ 550, 4.334498942e-01, 9.949501157e-01, 8.749999106e-03 },
		{ 555, 5.120500922e-01, 1.000000000e+00, 5.749999080e-03 },
		{ 560, 5.945000052e-01, 9.950000048e-01, 3.899999894e-03 },
		{ 565, 6.783999801e-01, 9.786000252e-01, 2.749999054e-03 },
		{ 570, 7.620999813e-01, 9.520000219e-01, 2.099999925e-03 },
		{ 575, 8.424999714e-01, 9.154000282e-01, 1.799999969e-03 },
		{ 580, 9.162999988e-01, 8.700000048e-01, 1.650001039e-03 },
		{ 585, 9.786000252e-01, 8.162999749e-01, 1.399999950e-03 },
		{ 590, 1.026299953e+00, 7.570000291e-01, 1.099999994e-03 },
		{ 595, 1.056699991e+00, 6.948999763e-01, 1.000000047e-03 },
		{ 600, 1.062199950e+00, 6.309999824e-01, 7.999999798e-04 },
		{ 605, 1.045600057e+00, 5.667999983e-01, 6.000000285e-04 },
		{ 610, 1.002599955e+00, 5.030000210e-01, 3.399999987e-04 },
		{ 615, 9.383999705e-01, 4.411999881e-01, 2.399999939e-04 },
		{ 620, 8.544499278e-01, 3.810000122e-01, 1.900000061e-04 },
		{ 625, 7.513999939e-01, 3.210000098e-01, 9.999999747e-05 },
		{ 630, 6.424000263e-01, 2.649999857e-01, 4.999999874e-05 },
		{ 635, 5.418999791e-01, 2.169999927e-01, 2.999999924e-05 },
		{ 640, 4.478999972e-01, 1.749999970e-01, 1.999999949e-05 },
		{ 645, 3.607999980e-01, 1.381999999e-01, 9.999999747e-06 },
		{ 650, 2.834999859e-01, 1.070000008e-01, 0.000000000e+00 },
		{ 655, 2.187000066e-01, 8.160000294e-02, 0.000000000e+00 },
		{ 660, 1.649000049e-01, 6.100000069e-02, 0.000000000e+00 },
		{ 665, 1.212000027e-01, 4.458000138e-02, 0.000000000e+00 },
		{ 670, 8.739999682e-02, 3.200000152e-02, 0.000000000e+00 },
		{ 675, 6.360000372e-02, 2.319999970e-02, 0.000000000e+00 },
		{ 680, 4.676999897e-02, 1.700000092e-02, 0.000000000e+00 },
		{ 685, 3.290000185e-02, 1.192000043e-02, 0.000000000e+00 },
		{ 690, 2.270000055e-02, 8.209999651e-03, 0.000000000e+00 },
		{ 695, 1.583999954e-02, 5.723000038e-03, 0.000000000e+00 },
		{ 700, 1.135915983e-02, 4.102000035e-03, 0.000000000e+00 },
		{ 705, 8.110916242e-03, 2.928999951e-03, 0.000000000e+00 },
		{ 710, 5.790345836e-03, 2.091000089e-03, 0.000000000e+00 },
		{ 715, 4.106456880e-03, 1.484000008e-03, 0.000000000e+00 },
		{ 720, 2.899327083e-03, 1.047000056e-03, 0.000000000e+00 },
		{ 725, 2.049189992e-03, 7.399999886e-04, 0.000000000e+00 },
		{ 730, 1.439971034e-03, 5.200000014e-04, 0.000000000e+00 } };

	// CIE RGB Conversion matrix
	float XYZ2RGB[3][3] = {
		{ (1219569.0 / 395920.0),     (-608687.0 / 395920.0),    (-107481.0 / 197960.0) },
		{ (-80960619.0 / 87888100.0), (82435961.0 / 43944050.0), (3976797.0 / 87888100.0) },
		{ (93813.0 / 1774030.0),      (-180961.0 / 887015.0),    (107481.0 / 93370.0) } };

	// For every pixel in our end image...
	for (int x = 0; x < specImg[0].img.rows; x++)
	{
		for(int y = 0; y < specImg[0].img.cols; y++)
		{
			float XYZ[3] = { 0 }; // Blank 3dim float array 

			// Wavelength Spectrum to XYZ Colorspace
			float Ysum = 0;
			for (int i = 0; i < 36; ++i) // Each pixel uses 36 samples (380nm to 730nm by 10's)
			{
				// i*2 = wavelength because colorMF is by 5nm, whereas specData is by 10nm
				// Add to XYZ[0] -> our CIE X color at [wavelength] * img[wavelength][x][y]
				Mat thisWave = getImage(i * 10 + 380);
				float intensity = static_cast<float>(thisWave.at<unsigned short>(x, y))/maxShort;
				XYZ[0] += colorMatchingFunc[i * 2][1] * intensity;
				XYZ[1] += colorMatchingFunc[i * 2][2] * intensity;
				XYZ[2] += colorMatchingFunc[i * 2][3] * intensity;
				Ysum += colorMatchingFunc[i * 2][2]; // Sum is merely Y's CIE color at [wavelength]
			}
			XYZ[0] /= Ysum; // Divide all by sum
			XYZ[1] /= Ysum;
			XYZ[2] /= Ysum;

			float colorPixel[3] = { 0 }; // Blank RGB pixel (in floating point)

			// Convert XYZ Colorspace to RGB for this pixel
			// Red = (const) * xyz[0] + (const) * xyz[1] + (const) * xyz[2]
			colorPixel[0] = XYZ2RGB[0][0] * XYZ[0] + XYZ2RGB[0][1] * XYZ[1] + XYZ2RGB[0][2] * XYZ[2];
			colorPixel[1] = XYZ2RGB[1][0] * XYZ[0] + XYZ2RGB[1][1] * XYZ[1] + XYZ2RGB[1][2] * XYZ[2];
			colorPixel[2] = XYZ2RGB[2][0] * XYZ[0] + XYZ2RGB[2][1] * XYZ[1] + XYZ2RGB[2][2] * XYZ[2];

			rgb(x, y)[2] = (unsigned char)(max(0.f, min(255.f, powf(colorPixel[0], 1 / gamma) * 255 + 0.5f)));
			rgb(x, y)[1] = (unsigned char)(max(0.f, min(255.f, powf(colorPixel[1], 1 / gamma) * 255 + 0.5f)));
			rgb(x, y)[0] = (unsigned char)(max(0.f, min(255.f, powf(colorPixel[2], 1 / gamma) * 255 + 0.5f)));
			
		}
	}

	return rgb;
}

/*
Creates a composite image by stacking three specific wavelength images ontop of each other
into a single composite image where the first, second, and third wavelength-images are
turned into the red, green, and blue channels of the compositie image respectively.
*/
Mat SpecImage::getComposite(int redWavelength, int blueWavelength, int greenWavelength)
{
	Mat redVal;
	Mat greenVal;
	Mat blueVal;

	vector<Mat> mergeArray;

	int Max = 256 * 16;
	int Min = 0;

	getImage(redWavelength).convertTo(redVal, CV_8U, 255.0 / (Max - Min), -255.0*Min / (Max - Min));
	getImage(blueWavelength).convertTo(greenVal, CV_8U, 255.0 / (Max - Min), -255.0*Min / (Max - Min));
	getImage(greenWavelength).convertTo(blueVal, CV_8U, 255.0 / (Max - Min), -255.0*Min / (Max - Min));

	mergeArray.push_back(blueVal);
	mergeArray.push_back(greenVal);
	mergeArray.push_back(redVal);

	Mat_<Vec3b> composite;

	//merge(mergeArray, composite); // This will not work in Release mode
	merge(&mergeArray[0], mergeArray.size(), composite);

	return composite;
}

/*
STATIC method to make a composite image given three grayscale images where
the first, second, and third images make up the red, blue, and green channels
of the composite image respectively.

NOTE - The returned pointer is created in this function and must be deleted by the
calling function.
*/
Mat SpecImage::makeComposite(Mat redImage, Mat blueImage, Mat greenImage)
{
	Mat redVal;
	Mat greenVal;
	Mat blueVal;

	vector<Mat> mergeArray;

	int Max = 256 * 16;
	int Min = 0;

	redImage.convertTo(redVal, CV_8U, 255.0 / (Max - Min), -255.0*Min / (Max - Min));
	blueImage.convertTo(greenVal, CV_8U, 255.0 / (Max - Min), -255.0*Min / (Max - Min));
	greenImage.convertTo(blueVal, CV_8U, 255.0 / (Max - Min), -255.0*Min / (Max - Min));

	mergeArray.push_back(blueVal);
	mergeArray.push_back(greenVal);
	mergeArray.push_back(redVal);

	Mat_<Vec3b> composite;

	//merge(mergeArray, composite); // This will not work in Release mode
	merge(&mergeArray[0], mergeArray.size(), composite);

	return composite;
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
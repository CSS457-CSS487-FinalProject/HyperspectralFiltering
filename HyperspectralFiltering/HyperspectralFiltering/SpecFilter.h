/*
SpecFilter represents a resonance pattern that spectral images
can be filtered against to determine where the target objects
are in a hyperspectral image.
*/
#pragma once
#include <string>
#include <vector>

using namespace std;

class SpecFilter
{
	public:
		SpecFilter();

		void LoadFromFile(string fileName);
		int GetIntensity(int wavelength) const; 
		void SetIntensity(int wavelength, int intensity);

	private:
		struct filterDataPoint
		{
			int wavelength;
			int intensity;
		};

		vector<filterDataPoint> filterData;
};
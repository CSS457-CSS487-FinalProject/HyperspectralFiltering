#pragma once
#include <string>
#include <vector>

using namespace std;

class SpecFilter
{
	public:
		SpecFilter(); 
		~SpecFilter(); // Will we need one?

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
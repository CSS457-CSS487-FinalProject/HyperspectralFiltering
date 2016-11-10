#include "SpecFilter.h"

SpecFilter::SpecFilter()
{
	// Create a blank filter
	filterData.push_back({0, 0});
}

void SpecFilter::LoadFromFile(std::string fileName)
{

}

int SpecFilter::GetIntensity(int wavelength) const
{
	return 0;
}

void SpecFilter::SetIntensity(int wavelength, int intensity)
{

}
#include "Utils.hpp"

void GenerateWaveClean(std::complex<SCALAR> Arr[], size_t size, float frequency)
{
	for (int i = 0; i < size; ++i)
	{
		Arr[i]._Val[0] = sinf(i * (frequency / SAMPLE_RATE));
	}
}

// We use sin here, cuz sin >>>>>>>> cos ;-)
void GenerateWaveClean(std::complex<SCALAR> Arr[], size_t size, float frequency, float shift)
{
	for (int i = 0; i < size; ++i) {
		Arr[i]._Val[0] = sin(((i*PI*2.0/ (DATA_SIZE)) * (frequency / SAMPLE_RATE)) + ((frequency > 0) ? shift : 0));
	}
}
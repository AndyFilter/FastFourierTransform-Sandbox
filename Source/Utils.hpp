#pragma once

#include <complex>
#include <iterator>
#include <cmath>
#include <valarray>
#include <algorithm>

#define SCALAR double
#define SAMPLE_RATE 1UL
#define POWER_2_DATA_SIZE (11) //You could use bigger sizes, but 2048 is pretty fast and is able to show what's important
#define DATA_SIZE (1 << POWER_2_DATA_SIZE) //2^n

const SCALAR PI = 3.1415926536;

extern void GenerateWaveClean(std::complex<SCALAR> Arr[], size_t size, float frequency);
extern void GenerateWaveClean(std::complex<SCALAR> Arr[], size_t size, float frequency, float shift);
inline unsigned int bitReverse(unsigned int x, int log2n);


// Faster algorithm than the one below. But is pretty much unreadable.
template<class Iter_T>
void fft(Iter_T a, Iter_T b, int log2n)
{
    typedef typename std::iterator_traits<Iter_T>::value_type complex;
    const complex J(0, 1);
    int n = 1 << log2n;
    for (unsigned int i = 0; i < n; ++i) {
        b[bitReverse(i, log2n)] = a[i];
    }
    for (int s = 1; s <= log2n; ++s) {
        int m = 1 << s;
        int m2 = m >> 1;
        complex w(1, 0);
        complex wm = exp(-J * (PI / m2));
        for (int j = 0; j < m2; ++j) {
            for (int k = j; k < n; k += m) {
                complex t = w * b[k + m2];
                complex u = b[k];
                b[k] = u + t;
                b[k + m2] = u - t;
            }
            w *= wm;
        }
    }
}

typedef std::complex<SCALAR> Complex;

// Cooley–Tukey FFT (in-place)
// Function changed a bit to better fit the program's needs
// But I don't like recursion, so (¬_¬) 
inline void fft_Cooley(Complex x[], size_t Size)
{
    const size_t N = Size;
    if (N <= 1) return;

    // divide
    Complex* even = new Complex[N / 2];
    Complex*  odd = new Complex[N / 2];

    int count = 0;
    std::copy_if(x, x + Size, even, [&count](const Complex i) { return (count++) % 2 == 0; });
    count = 0;
    std::copy_if(x, x + Size, odd, [&count](const Complex i) {return (count++) % 2 == 1; });

    // conquer
    fft_Cooley(even, N/2);
    fft_Cooley(odd, N/2);

    // combine
    for (size_t k = 0; k < N / 2; ++k)
    {
        Complex t = std::polar<SCALAR>(1.0, -2 * PI * k / N) * odd[k];
        x[k] = even[k] + t;
        x[k + N / 2] = even[k] - t;
    }

    // clean-up
    delete[] odd;
    delete[] even;
}

unsigned int bitReverse(unsigned int x, int log2n) {
    int n = 0;
    int mask = 0x1;
    for (int i = 0; i < log2n; i++) {
        n <<= 1;
        n |= (x & 1);
        x >>= 1;
    }
    return n;
}
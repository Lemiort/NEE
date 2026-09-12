#include "PerlinNoise.hpp"

PerlinNoise::PerlinNoise()
    : persistence(0), frequency(0), amplitude(0), octaves(0), randomseed(0) {}

PerlinNoise::PerlinNoise(double _persistence, double _frequency,
                         double _amplitude, int _octaves, int _randomseed)
    : persistence(_persistence),
      frequency(_frequency),
      amplitude(_amplitude),
      octaves(_octaves),
      randomseed(2 + _randomseed * _randomseed) {
    // strength, frequency

    // frequency

    // amplitude

    // octave
}

void PerlinNoise::Set(double _persistence, double _frequency, double _amplitude,
                      int _octaves, int _randomseed) {
    persistence = _persistence;
    frequency = _frequency;
    amplitude = _amplitude;
    octaves = _octaves;
    randomseed = 2 + (_randomseed * _randomseed);
}

double PerlinNoise::GetHeight(double x, double y) const {
    return amplitude * Total(x, y);
}

double PerlinNoise::Total(double i, double j) const {
    // properties of one octave (changing each loop)
    double t = 0.0F;
    double _amplitude = 1;
    double freq = frequency;

    for (int k = 0; k < octaves; k++) {
        t += GetValue((j * freq) + randomseed, (i * freq) + randomseed) *
             _amplitude;
        _amplitude *= persistence;
        freq *= 2;
    }

    return t;
}

double PerlinNoise::GetValue(double x, double y) const {
    int const Xint = static_cast<int>(x);
    int const Yint = static_cast<int>(y);
    double const Xfrac = x - Xint;
    double const Yfrac = y - Yint;

    // noise values
    double const n01 = Noise(Xint - 1, Yint - 1);
    double const n02 = Noise(Xint + 1, Yint - 1);
    double const n03 = Noise(Xint - 1, Yint + 1);
    double const n04 = Noise(Xint + 1, Yint + 1);
    double const n05 = Noise(Xint - 1, Yint);
    double const n06 = Noise(Xint + 1, Yint);
    double const n07 = Noise(Xint, Yint - 1);
    double const n08 = Noise(Xint, Yint + 1);
    double const n09 = Noise(Xint, Yint);

    double const n12 = Noise(Xint + 2, Yint - 1);
    double const n14 = Noise(Xint + 2, Yint + 1);
    double const n16 = Noise(Xint + 2, Yint);

    double const n23 = Noise(Xint - 1, Yint + 2);
    double const n24 = Noise(Xint + 1, Yint + 2);
    double const n28 = Noise(Xint, Yint + 2);

    double const n34 = Noise(Xint + 2, Yint + 2);

    // find the noise values of the four corners
    double const x0y0 = (0.0625 * (n01 + n02 + n03 + n04)) +
                        (0.125 * (n05 + n06 + n07 + n08)) + (0.25 * n09);
    double const x1y0 = (0.0625 * (n07 + n12 + n08 + n14)) +
                        (0.125 * (n09 + n16 + n02 + n04)) + (0.25 * n06);
    double const x0y1 = (0.0625 * (n05 + n06 + n23 + n24)) +
                        (0.125 * (n03 + n04 + n09 + n28)) + (0.25 * n08);
    double const x1y1 = (0.0625 * (n09 + n16 + n28 + n34)) +
                        (0.125 * (n08 + n14 + n06 + n24)) + (0.25 * n04);

    // interpolate between those values according to the x and y fractions
    double const v1 =
        Interpolate(x0y0, x1y0, Xfrac);  // interpolate in x direction (y)
    double const v2 =
        Interpolate(x0y1, x1y1, Xfrac);  // interpolate in x direction (y+1)
    double const fin =
        Interpolate(v1, v2, Yfrac);  // interpolate in y direction

    return fin;
}

double PerlinNoise::Interpolate(double x, double y, double a) const {
    double const negA = 1.0 - a;
    double const negASqr = negA * negA;
    double const fac1 = (3.0 * negASqr) - (2.0 * (negASqr * negA));
    double const aSqr = a * a;
    double const fac2 = (3.0 * aSqr) - (2.0 * (aSqr * a));

    return (x * fac1) + (y * fac2);  // add the weighted factors
}

double PerlinNoise::Noise(int x, int y) const {
    int n = x + (y * 57);
    n = (n << 13) ^ n;
    int const t = ((n * ((n * n * 15731) + 789221)) + 1376312589) & 0x7fffffff;
    return 1.0 - (static_cast<double>(t) *
                  0.931322574615478515625e-9);  /// 1073741824.0);
}

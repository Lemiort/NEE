#ifndef PERLINNOISE_H_INCLUDED
#define PERLINNOISE_H_INCLUDED

class PerlinNoise {
public:
    // Constructor
    PerlinNoise();
    PerlinNoise(double _persistence, double _frequency, double _amplitude,
                int _octaves, int _randomseed);

    // Get Height
    [[nodiscard]] double GetHeight(double x, double y) const;

    // Get
    [[nodiscard]] double Persistence() const { return persistence; }
    [[nodiscard]] double Frequency() const { return frequency; }
    [[nodiscard]] double Amplitude() const { return amplitude; }
    [[nodiscard]] int Octaves() const { return octaves; }
    [[nodiscard]] int RandomSeed() const { return randomseed; }

    // Set
    void Set(double _persistence, double _frequency, double _amplitude,
             int _octaves, int _randomseed);

    void SetPersistence(double _persistence) { persistence = _persistence; }
    void SetFrequency(double _frequency) { frequency = _frequency; }
    void SetAmplitude(double _amplitude) { amplitude = _amplitude; }
    void SetOctaves(int _octaves) { octaves = _octaves; }
    void SetRandomSeed(int _randomseed) { randomseed = _randomseed; }

private:
    [[nodiscard]] double Total(double i, double j) const;
    [[nodiscard]] double GetValue(double x, double y) const;
    [[nodiscard]] static double Interpolate(double x, double y, double a);
    [[nodiscard]] static double Noise(int x, int y);

    double persistence, frequency, amplitude;
    int octaves, randomseed;
};

#endif  // PERLINNOISE_H_INCLUDED

#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

// Streaming stereo statistics. RMS is intentionally not labelled LUFS.
struct Analysis
{
    double energy = 0, midEnergy = 0, sideEnergy = 0, cross = 0, leftEnergy = 0, rightEnergy = 0;
    float peak = 0;
    uint64_t samples = 0;
    void push(float l, float r)
    {
        const double m = (double(l) + r) * .5, s = (double(l) - r) * .5;
        energy += (double(l)*l + double(r)*r)*.5;
        midEnergy += m*m; sideEnergy += s*s; cross += double(l)*r;
        leftEnergy += double(l)*l; rightEnergy += double(r)*r;
        peak = std::max(peak, std::max(std::abs(l), std::abs(r))); ++samples;
    }
    float rmsDb() const { return float(10*std::log10(std::max(1.e-12, energy / std::max(uint64_t(1), samples)))); }
    float peakDb() const { return 20*std::log10(std::max(1.e-6f, peak)); }
    float sideRatio() const { return float(sideEnergy / std::max(1.e-12, midEnergy + sideEnergy)); }
    float correlation() const { return float(cross / std::max(1.e-12, std::sqrt(leftEnergy*rightEnergy))); }
};

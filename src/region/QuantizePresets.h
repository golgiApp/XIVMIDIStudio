#pragma once
#include "MidiRegionModel.h"
#include <array>
namespace xiv {
inline constexpr std::array<const char *, 7> quantizeNames{
    "Quarter", "Eighth", "Sixteenth", "Eighth Triplet", "Sixteenth Triplet",
    "Eighth Swing", "Sixteenth Swing"};
inline RegionQuantize quantizePreset(int index, int ppq) {
    constexpr int divisors[] = {1, 2, 4, 3, 6, 2, 4};
    if (index < 0 || index >= 7 || ppq <= 0) return {};
    return {std::max(1, (ppq + divisors[index] / 2) / divisors[index]), 100, index >= 5 ? 66 : 50};
}
}

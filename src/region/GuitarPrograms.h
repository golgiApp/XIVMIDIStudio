#pragma once
#include "MidiRegionRender.h"
#include <array>
namespace xiv {
inline constexpr std::array<int, 5> guitarPrograms{29, 27, 28, 30, 31};
inline constexpr std::array<const char *, 5> guitarNames{
    "I · Overdriven", "II · Clean", "III · Muted", "IV · Power Chords", "V · Special"};
inline bool isGuitarProgram(int program) {
    return std::find(guitarPrograms.begin(), guitarPrograms.end(), program) != guitarPrograms.end();
}
inline int guitarProgramOrDefault(int program) { return isGuitarProgram(program) ? program : -1; }
inline bool guitarConflict(const OwnedRegionNote &x, const OwnedRegionNote &y) {
    return (isGuitarProgram(x.program) || isGuitarProgram(y.program)) &&
        x.channel == y.channel && x.program != y.program &&
        x.startTick < y.endTick && y.startTick < x.endTick;
}
inline std::set<int> conflictingGuitarNotes(const std::vector<OwnedRegionNote> &notes) {
    std::set<int> result;
    for (std::size_t a = 0; a < notes.size(); ++a)
        for (std::size_t b = a + 1; b < notes.size(); ++b)
            if (guitarConflict(notes[a], notes[b])) {
                result.insert(notes[a].id); result.insert(notes[b].id);
            }
    return result;
}
enum class GuitarConflictChoice { Highest, Lowest, Shorten };
inline bool resolveGuitarConflicts(std::vector<OwnedRegionNote> &notes, GuitarConflictChoice choice) {
    auto ordered = notes;
    if (choice == GuitarConflictChoice::Shorten) {
        std::stable_sort(ordered.begin(), ordered.end(), [](auto &a, auto &b) { return a.startTick < b.startTick; });
        for (std::size_t a = 0; a < ordered.size(); ++a)
            for (std::size_t b = a + 1; b < ordered.size(); ++b) {
                auto &x = ordered[a]; const auto &y = ordered[b];
                if (!guitarConflict(x, y)) continue;
                if (x.startTick == y.startTick) return false;
                x.endTick = std::max(x.startTick + 1, y.startTick - 8);
            }
    } else {
        std::stable_sort(ordered.begin(), ordered.end(), [choice](auto &a, auto &b) {
            return choice == GuitarConflictChoice::Highest ? a.pitch > b.pitch : a.pitch < b.pitch;
        });
        std::vector<OwnedRegionNote> kept;
        for (const auto &note : ordered)
            if (std::none_of(kept.begin(), kept.end(), [&](const auto &other) { return guitarConflict(note, other); }))
                kept.push_back(note);
        ordered = std::move(kept);
    }
    notes = std::move(ordered);
    return true;
}
}

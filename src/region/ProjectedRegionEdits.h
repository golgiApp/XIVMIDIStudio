#ifndef XIV_PROJECTED_REGION_EDITS_H
#define XIV_PROJECTED_REGION_EDITS_H

#include "MidiRegionRender.h"

namespace xiv {

// Selection IDs are one-based indexes in the complete rendered schedule.
// This distinguishes repeated notes and equal note IDs in separate regions.
inline bool materializeProjectedRegionIds(MidiRegionModel &model,
                                          const std::vector<RenderedRegionNote> &rendered,
                                          const std::set<int> &regions, int trackCount) {
    if (regions.empty()) return false;
    for (int id : regions)
        if (!model.find(id)) return false;

    auto records = model.regions();
    for (auto &region : records) {
        if (!regions.count(region.id)) continue;
        std::vector<OwnedRegionNote> notes;
        for (std::size_t i = 0; i < rendered.size(); ++i) {
            const auto &note = rendered[i];
            if (note.regionId != region.id) continue;
            notes.push_back({static_cast<int>(i) + 1, note.startTick - region.startTick,
                note.endTick - region.startTick, note.pitch, note.velocity, note.channel, note.program});
        }
        region.sourceEndTick = region.displayDuration();
        region.sourceStartTick = 0;
        region.loopLengthTicks = 0;
        region.quantize = {};
        region.program = -1;
        region.startNoteMode = CrossingNoteMode::Split;
        region.endNoteMode = CrossingNoteMode::Keep;
        region.ownedNotes = std::make_shared<const std::vector<OwnedRegionNote>>(std::move(notes));
    }
    return model.replaceValidated(records, trackCount);
}


inline bool materializeProjectedRegions(MidiRegionModel &model,
                                       const std::vector<RenderedRegionNote> &rendered,
                                       const std::set<int> &selected, int trackCount) {
    std::set<int> regions;
    for (int id : selected) {
        if (id <= 0 || id > static_cast<int>(rendered.size())) return false;
        regions.insert(rendered[id - 1].regionId);
    }
    return materializeProjectedRegionIds(model, rendered, regions, trackCount);
}

// Fine nudging bakes the current rendered timing so active quantization cannot
// round a one-tick edit back to its previous position. Commit only on success.
inline bool nudgeProjectedNotes(MidiRegionModel &model,
                                const std::vector<RenderedRegionNote> &rendered,
                                const std::set<int> &selected, int ticks, int trackCount) {
    if (ticks == 0 || selected.empty()) return false;
    auto draft = model;
    if (!materializeProjectedRegions(draft, rendered, selected, trackCount)) return false;
    std::set<int> affected;
    for (int id : selected) affected.insert(rendered[id - 1].regionId);
    for (int id : affected) {
        const auto *region = draft.find(id);
        auto notes = *region->ownedNotes;
        for (auto &note : notes) if (selected.count(note.id)) {
            const auto start = static_cast<long long>(note.startTick) + ticks;
            const auto end = static_cast<long long>(note.endTick) + ticks;
            if (start < 0 || end > region->duration()) return false;
            note.startTick = static_cast<int>(start);
            note.endTick = static_cast<int>(end);
        }
        if (!draft.replaceOwnedNotes(id, std::move(notes))) return false;
    }
    model = std::move(draft);
    return true;
}

} // namespace xiv
#endif

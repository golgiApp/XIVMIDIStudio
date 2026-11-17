#ifndef MIDI_REGION_TRANSFER_H
#define MIDI_REGION_TRANSFER_H

#include "MidiRegionRender.h"

namespace xiv {

inline bool validRegionDestination(int track, int start, int64_t end, int trackCount) {
    return track >= 0 && track < trackCount && start >= 0 && end > start &&
           end <= std::numeric_limits<int>::max();
}

inline int emptyRegionGapDuration(const MidiRegionModel &model, int track, int start, int limit) {
    if (start < 0 || limit <= start || model.at(track, start)) return 0;
    for (const auto &region : model.regions()) {
        if (region.trackIndex == track && region.startTick > start)
            limit = std::min(limit, region.startTick);
    }
    return limit - start;
}

struct RegionClipboard {
    int duration = 0;
    std::vector<OwnedRegionNote> notes;
    std::string name;
    int program = -1;
};

inline int insertOwnedRegion(MidiRegionModel &model, int track, int start, int trackCount,
                             const RegionClipboard &clip, int channel) {
    const int64_t end = static_cast<int64_t>(start) + clip.duration;
    if (clip.duration <= 0 || !validRegionDestination(track, start, end, trackCount) ||
        channel < 0 || channel >= 16) return 0;
    auto notes = clip.notes;
    for (auto &note : notes) {
        if (static_cast<int64_t>(start) + note.endTick > std::numeric_limits<int>::max())
            return 0;
        note.channel = channel;
    }
    if (!validOwnedNotes(notes)) return 0;

    const int id = model.addInitial(track, 0, clip.duration);
    model.move(id, start);
    model.replaceOwnedNotes(id, std::move(notes));
    model.rename(id, clip.name);
    if (clip.program >= 0) model.setProgram(id, clip.program);
    return id;
}

// Append clipboard notes without shifting any existing material.
inline bool appendRegionNotesAtPlayhead(const MidiRegion &region, int playhead, int channel,
                                       const std::vector<OwnedRegionNote> &clipboard,
                                       std::vector<OwnedRegionNote> &destination) {
    if (clipboard.empty() || !region.contains(playhead) || channel < 0 || channel > 15)
        return false;
    int first = std::numeric_limits<int>::max();
    for (const auto &note : clipboard) first = std::min(first, note.startTick);
    const auto offset = static_cast<int64_t>(region.sourceStartTick) +
                        playhead - region.startTick - first;
    int64_t nextId = 1;
    for (const auto &note : destination)
        nextId = std::max(nextId, static_cast<int64_t>(note.id) + 1);
    std::vector<OwnedRegionNote> pasted;
    pasted.reserve(clipboard.size());
    for (auto note : clipboard) {
        const auto start = note.startTick + offset;
        const auto end = note.endTick + offset;
        if (start < region.sourceStartTick || end <= start ||
            end > region.sourceEndTick || nextId > std::numeric_limits<int>::max())
            return false;
        note.id = static_cast<int>(nextId++);
        note.startTick = static_cast<int>(start);
        note.endTick = static_cast<int>(end);
        note.channel = channel;
        pasted.push_back(note);
    }
    if (!validOwnedNotes(pasted)) return false;
    destination.insert(destination.end(), pasted.begin(), pasted.end());
    return true;
}

// Capture the audible note positions before removing anything. The resulting
// region owns its notes independently of the original tracks and transforms.
inline bool consolidateRegions(MidiRegionModel &model, const std::set<int> &ids,
                               int primaryId, int destinationTrack, int destinationStart,
                               int destinationChannel, int trackCount,
                               const std::vector<SourceRegionNote> &source) {
    const auto *primary = model.find(primaryId);
    if (!primary || !ids.count(primaryId)) return false;
    int first = std::numeric_limits<int>::max();
    int last = 0;
    bool changesTrack = false;
    for (int id : ids) {
        const auto *region = model.find(id);
        if (!region) return false;
        first = std::min(first, region->startTick);
        last = std::max(last, region->endTick());
        changesTrack |= region->trackIndex != destinationTrack;
    }
    if (changesTrack && (destinationChannel < 0 || destinationChannel > 15)) return false;
    const int64_t end = static_cast<int64_t>(destinationStart) + (last - first);
    if (!validRegionDestination(destinationTrack, destinationStart, end, trackCount))
        return false;

    std::vector<OwnedRegionNote> notes;
    for (const auto &note : renderRegionNotes(source, model)) {
        if (!ids.count(note.regionId)) continue;
        const int64_t noteEnd = static_cast<int64_t>(destinationStart) + note.endTick - first;
        if (noteEnd > std::numeric_limits<int>::max()) return false;
        notes.push_back({static_cast<int>(notes.size()) + 1, note.startTick - first,
                         note.endTick - first, note.pitch, note.velocity,
                         note.trackIndex == destinationTrack ? note.channel : destinationChannel, note.program});
    }

    MidiRegion result{primaryId, destinationTrack, destinationStart, 0, last - first};
    result.name = primary->name;
    result.program = ids.size() == 1 ? primary->program : -1;
    result.ownedNotes = std::make_shared<const std::vector<OwnedRegionNote>>(std::move(notes));
    std::vector<MidiRegion> regions;
    for (const auto &region : model.regions())
        if (!ids.count(region.id)) regions.push_back(region);
    regions.push_back(std::move(result));
    return model.replaceValidated(regions, trackCount);
}

} // namespace xiv

#endif

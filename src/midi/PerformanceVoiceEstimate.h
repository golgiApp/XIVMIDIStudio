#pragma once
#include <fluidsynth.h>
#include <set>
#include <vector>
namespace xiv {
// Preview note-on groups, including release phase. Not the game's voice allocator.
inline std::set<unsigned int> soundingNoteIds(fluid_synth_t *synth) {
    std::set<unsigned int> result;
    if(!synth) return result;
    std::vector<fluid_voice_t*> voices(fluid_synth_get_polyphony(synth),nullptr);
    fluid_synth_get_voicelist(synth,voices.data(),int(voices.size()),-1);
    for(const auto *voice:voices) {
        if(!voice) break;
        if(fluid_voice_is_playing(voice)) result.insert(fluid_voice_get_id(voice));
    }
    return result;
}
inline int estimatedVoiceCount(fluid_synth_t *synth, std::set<unsigned int> &excluded) {
    auto notes=soundingNoteIds(synth);
    for(auto it=excluded.begin();it!=excluded.end();) {
        if(!notes.count(*it)) it=excluded.erase(it);
        else { notes.erase(*it); ++it; }
    }
    return int(notes.size());
}
}

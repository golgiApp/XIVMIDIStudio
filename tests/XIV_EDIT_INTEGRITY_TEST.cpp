#include "../src/region/ProjectedRegionEdits.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace xiv;
    MidiRegionModel model;
    const int a = model.addInitial(0, 0, 480);
    const int b = model.addInitial(1, 0, 480);
    const int empty = model.addInitial(2, 0, 480);
    const std::vector<SourceRegionNote> original{
        {1,0,0,120,48,100,0}, {2,0,0,120,60,100,0},
        {3,0,0,120,72,100,0}, {4,1,0,120,96,100,1},
        {5,2,0,120,60,100,2}};
    assert(materializeProjectedRegionIds(model, renderRegionNotes(original,model), {a,b}, 3));
    auto notes = *model.find(a)->ownedNotes;
    notes.erase(notes.begin(), notes.end()-1); // keep highest voice
    notes.front().pitch -= 12; // subsequent transposition
    assert(model.replaceOwnedNotes(a, notes));
    assert(model.replaceOwnedNotes(b, {})); // delete all notes
    assert(model.remove(empty)); // deleted last region must remain silent
    const auto edited = model;
    assert(model.remapTracks({2,0,1},3)); // reorder existing tracks
    auto reorderedSource=original;
    for(auto &n:reorderedSource) n.trackIndex=std::vector<int>{2,0,1}[n.trackIndex];
    auto rendered=renderRegionNotes(reorderedSource,model);
    assert(rendered.size()==1 && rendered[0].pitch==60 && rendered[0].trackIndex==2);
    assert(model.isManagedTrack(1));
    // Merge the edited track into the empty destination, then remove source.
    assert(materializeProjectedRegionIds(model,rendered,{a,b},3));
    assert(model.remapTracks({0,1,0},3));
    assert(model.remapTracks({0,1,-1},2));
    for(auto &n:reorderedSource) if(n.trackIndex==2) n.trackIndex=0;
    rendered=renderRegionNotes(reorderedSource,model);
    assert(rendered.size()==1 && rendered[0].pitch==60 && rendered[0].trackIndex==0);
    // Restoring a snapshot and repeating the edit never mutates that snapshot.
    auto restored=edited;
    rendered=renderRegionNotes(original,restored);
    assert(rendered.size()==1 && rendered[0].pitch==60);
    assert(restored.find(b)->ownedNotes && restored.find(b)->ownedNotes->empty());
    assert(restored.isManagedTrack(2));
    std::cout << "Edited notes survive reorder, merge, deletion, and snapshot restore\n";
}

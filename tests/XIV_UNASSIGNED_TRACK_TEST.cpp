#include "../src/region/MidiRegionTransfer.h"
#include <cassert>
int main() {
    xiv::MidiRegionModel model;
    const int original = model.addInitial(0, 0, 100);
    const std::vector<xiv::SourceRegionNote> source{{1,0,10,30,60,90,3},{2,0,40,70,64,80,3}};
    // A destination above track 16 is legal, even without a playback channel.
    assert(xiv::consolidateRegions(model,{original},original,24,200,-1,25,source));
    auto notes = xiv::renderRegionNotes(source, model);
    assert(notes.size()==2 && notes[0].trackIndex==24 && notes[0].startTick==210);
    assert(notes[0].channel==3 && notes[0].pitch==60 && notes[1].velocity==80);
    const auto snapshot=model;
    xiv::RegionClipboard clip{100,{{1,0,20,67,100,5}},"Working"};
    const int pasted=xiv::insertOwnedRegion(model,24,400,25,clip,-1);
    assert(pasted && model.find(pasted)->ownedNotes->front().channel==5);
    std::vector<xiv::OwnedRegionNote> combined=*model.find(pasted)->ownedNotes;
    assert(xiv::appendRegionNotesAtPlayhead(*model.find(pasted),430,-1,clip.notes,combined));
    assert(combined.size()==2 && combined.back().startTick==30 && combined.back().channel==5);
    assert(model.replaceOwnedNotes(pasted,combined));
    assert(model.reassignOwnedNoteChannels({{24,7}}));
    notes=xiv::renderRegionNotes(source,model);
    assert(notes.size()==4);
    for(const auto &n:notes) assert(n.channel==7);
    assert(xiv::consolidateRegions(model,{original,pasted},original,2,0,6,25,source));
    notes=xiv::renderRegionNotes(source,model);
    assert(notes.size()==4);
    for(const auto &n:notes) assert(n.trackIndex==2 && n.channel==6);
    // Restoring the prior model restores the original unassigned-track contents.
    model=snapshot;
    notes=xiv::renderRegionNotes(source,model);
    assert(notes.size()==2 && notes[0].trackIndex==24 && notes[0].channel==3);
    assert(!xiv::insertOwnedRegion(model,25,0,25,clip,-1));
    assert(!xiv::insertOwnedRegion(model,24,0,25,clip,-2));
}

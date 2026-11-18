#include "../src/region/MidiRegionTransfer.h"
#include <cassert>
int main(){
 xiv::MidiRegionModel m;
 std::vector<xiv::RegionClipboardItem> group{{0,0,{96,{{1,0,48,60,91,0,27}},"Lead",27}},{2,48,{96,{{1,0,24,64,85,2}},"Harmony",-1}}};
 std::vector<int> channels{0,1,-1,3,4};
 auto ids=xiv::pasteRegionGroup(m,group,0,480,channels);assert(ids.size()==2);
 auto notes=xiv::renderRegionNotes({},m);assert(notes.size()==2);
 assert(notes[0].startTick==480 && notes[0].trackIndex==0 && notes[0].program==27);
 assert(notes[1].startTick==528 && notes[1].trackIndex==2 && notes[1].channel==2);
 auto snapshot=m;auto bad=xiv::pasteRegionGroup(m,group,4,0,channels);assert(bad.empty());
 assert(m.regions().size()==snapshot.regions().size());
 auto again=xiv::pasteRegionGroup(m,group,1,960,channels);assert(again.size()==2);
 assert(m.find(again[0])->trackIndex==1 && m.find(again[1])->trackIndex==3);
 assert(m.find(again[0])->ownedNotes->front().channel==1);
 assert(m.find(again[1])->ownedNotes->front().channel==3);
}

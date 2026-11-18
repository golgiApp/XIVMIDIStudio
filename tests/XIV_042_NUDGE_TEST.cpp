#include "../src/region/ProjectedRegionEdits.h"
#include <cassert>
int main() {
 xiv::MidiRegionModel model;
 const int a=model.addInitial(0,0,100), b=model.addInitial(1,0,100);
 assert(model.replaceOwnedNotes(a,{{1,23,43,60,90,0},{2,60,80,64,80,0}}));
 assert(model.replaceOwnedNotes(b,{{1,23,43,67,90,0}})); // Deliberately shared channel.
 assert(model.setQuantize(a,{25,100,50}));
 auto before=xiv::renderRegionNotes({},model);
 std::set<int> chosen;
 for(unsigned i=0;i<before.size();++i) if(before[i].pitch!=64) chosen.insert(i+1);
 assert(chosen.size()==2);
 assert(xiv::nudgeProjectedNotes(model,before,chosen,1,2));
 auto after=xiv::renderRegionNotes({},model);
 assert(after.size()==before.size());
 for(unsigned i=0;i<after.size();++i){
   assert(after[i].startTick==before[i].startTick+(chosen.count(i+1)?1:0));
   assert(after[i].endTick-after[i].startTick==before[i].endTick-before[i].startTick);
   assert(after[i].channel==0);
 }
 // Boundary failure is atomic for every affected track.
 auto snapshot=model;
 assert(!xiv::nudgeProjectedNotes(model,after,chosen,-1000,2));
 for(const auto&r:model.regions()) assert(r.ownedNotes==snapshot.find(r.id)->ownedNotes);
 assert(!xiv::nudgeProjectedNotes(model,after,{999},1,2));
 assert(xiv::nudgeProjectedNotes(model,after,chosen,-1,2));
 auto restored=xiv::renderRegionNotes({},model);
 for(unsigned i=0;i<before.size();++i)assert(restored[i].startTick==before[i].startTick);
 // A single loop occurrence can be nudged without moving its siblings.
 xiv::MidiRegionModel loop;
 int r=loop.addInitial(0,0,100);
 assert(loop.replaceOwnedNotes(r,{{1,20,40,60,90,0}}));
 assert(loop.loopTo(r,300));
 auto repeats=xiv::renderRegionNotes({},loop);assert(repeats.size()==3);
 assert(xiv::nudgeProjectedNotes(loop,repeats,{2},1,1));
 auto changed=xiv::renderRegionNotes({},loop);
 assert(changed[0].startTick==20 && changed[1].startTick==121 && changed[2].startTick==220);
}

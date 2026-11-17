#include "../src/region/GuitarPrograms.h"
#include "../src/region/MidiRegionTransfer.h"
#include "../src/region/ProjectedRegionEdits.h"
#include "../src/region/QuantizePresets.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace xiv;
    MidiRegionModel model;
    const int a = model.addInitial(0, 0, 480);
    const int b = model.addInitial(0, 480, 960);
    std::vector<SourceRegionNote> source{{1,0,0,120,60,100,0,27},{2,0,480,600,64,90,0,27}};
    model.setProgram(a,29); model.setProgram(b,31);
    assert(!model.trackIsIdentity(0,960));
    auto rendered=renderRegionNotes(source,model);
    assert(rendered.size()==2 && rendered[0].program==29 && rendered[1].program==31);
    const auto before=model;
    auto splitModel=model;
    const int right=splitModel.split(a,240,CrossingNoteMode::Split);
    assert(right && splitModel.find(right)->program==29);
    const int duplicate=splitModel.duplicate(b,1440);
    assert(duplicate && splitModel.find(duplicate)->program==31);
    assert(consolidateRegions(model,{a,b},a,0,0,0,1,source));
    rendered=renderRegionNotes(source,model);
    assert(rendered.size()==2 && rendered[0].program==29 && rendered[1].program==31);
    assert(conflictingGuitarNotes(*model.find(a)->ownedNotes).empty());
    assert(model.loopTo(a,1920));
    rendered=renderRegionNotes(source,model);
    assert(rendered.size()==4 && rendered[2].program==29 && rendered[3].program==31);
    auto materialized=model;
    assert(materializeProjectedRegionIds(materialized,rendered,{a},1));
    auto copied=renderRegionNotes({},materialized);
    assert(copied.size()==4 && copied[3].program==31);
    materialized.setProgram(a,-1);
    assert(materialized.find(a)->program==-1);
    for(auto &n:*materialized.find(a)->ownedNotes) assert(n.program==-1);
    model=before;
    assert(model.find(a)->program==29 && model.find(b)->program==31);
    std::vector<OwnedRegionNote> overlap{{1,0,240,60,100,0,29},{2,120,360,72,100,0,31},{3,400,450,55,100,0,29}};
    assert(conflictingGuitarNotes(overlap)==std::set<int>({1,2}));
    auto highest=overlap;
    assert(resolveGuitarConflicts(highest,GuitarConflictChoice::Highest));
    assert(highest.size()==2 && conflictingGuitarNotes(highest).empty());
    auto lowest=overlap;
    assert(resolveGuitarConflicts(lowest,GuitarConflictChoice::Lowest));
    assert(lowest.size()==2 && conflictingGuitarNotes(lowest).empty());
    assert(resolveGuitarConflicts(overlap,GuitarConflictChoice::Shorten));
    assert(overlap[0].endTick==112 && overlap.size()==3);
    overlap[1].startTick=0;
    auto snapshot=overlap;
    assert(!resolveGuitarConflicts(overlap,GuitarConflictChoice::Shorten));
    assert(overlap[0].endTick==snapshot[0].endTick);
    MidiRegionModel empty;
    int emptyId = empty.addInitial(0,0,480);
    empty.setProgram(emptyId,30);
    assert(consolidateRegions(empty,{emptyId},emptyId,0,0,0,1,{}));
    assert(empty.find(emptyId)->program==30);
    RegionClipboard clipboard{480,{},"Silent guitar",30};
    int pasted=insertOwnedRegion(empty,0,480,1,clipboard,0);
    assert(pasted && empty.find(pasted)->program==30);
    const int expected[]{480,240,120,160,80,240,120};
    for(int i=0;i<7;++i) {
        auto q=quantizePreset(i,480);
        assert(q.gridTicks==expected[i] && q.strength==100 && q.swing==(i>=5?66:50));
    }
    MidiRegion region{1,0,0,0,1920};
    region.quantize=quantizePreset(5,480);
    int64_t start=240,end=360;
    quantizeRegionNote(region,start,end);
    assert(start==317 && end-start==120);
    region.quantize=quantizePreset(3,480);start=170;end=230;
    quantizeRegionNote(region,start,end);
    assert(start==160 && end==220);
    std::cout << "Guitar sound preservation, conflicts, reset, loops, and all seven quantize presets passed\n";
}

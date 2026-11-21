#include "../src/gui/RegionClipboardIO.h"
#include "../src/support/ExportNaming.h"
#include <cassert>
#include <iostream>
int main(int argc,char**argv) {
 QApplication app(argc,argv);
 const std::vector<xiv::RegionClipboardItem> original={
  {0,0,{480,{{1,0,240,60,90,2,27}},"Lead",27}},
  {2,240,{480,{{1,0,480,72,80,4,-1}},"Harmony",-1}}};
 auto bytes=xiv::encodeRegionClipboard(original,480);
 auto decoded=xiv::decodeRegionClipboard(bytes,960);
 assert(decoded.size()==2 && decoded[1].tickOffset==480 && decoded[0].clip.duration==960);
 assert(decoded[1].clip.notes[0].endTick==960 && decoded[0].clip.program==27);
 xiv::MidiRegionModel target; std::vector<int> channels={0,1,2,3};
 auto pasted=xiv::pasteRegionGroup(target,decoded,1,1920,channels);
 assert(pasted.size()==2 && target.find(pasted[1])->trackIndex==3);
 assert(target.find(pasted[0])->startTick==1920 && target.find(pasted[1])->startTick==2400);
 assert(target.find(pasted[1])->ownedNotes->at(0).channel==3);
 const auto oldSize=target.regions().size();
 assert(xiv::pasteRegionGroup(target,decoded,3,1920,channels).empty());
 assert(target.regions().size()==oldSize);
 xiv::setClipboardPayload(xiv::regionMime,bytes);
 xiv::setClipboardPayload("application/x-midieditor-events-v2",QByteArray("note data"));
 assert(xiv::systemRegionClipboard(480).size()==2);
 assert(QApplication::clipboard()->mimeData()->data("application/x-midieditor-events-v2")=="note data");
 assert(xiv::decodeRegionClipboard("bad json",480).empty());
 auto malformed=QJsonDocument::fromJson(bytes).object(); malformed["tpq"]=0;
 assert(xiv::decodeRegionClipboard(QJsonDocument(malformed).toJson(),480).empty());
 assert(xiv::exportBaseName("{editor} {players}P {artist} - {title}","April in Paris","Count Basie","GA",8)=="GA 8P Count Basie - April in Paris");
 assert(xiv::exportBaseName("{title}","Song.mid","","",0)=="Song");
 assert(xiv::exportBaseName("{title}","CON","","",0)=="_CON");
 assert(xiv::exportBaseName("{title}","A/B:C?","","",0)=="A_B_C_");
 std::cout<<"Clipboard: cross-project scaling, track spacing, routing, atomic failure, independent payloads; export naming PASS\n";
}

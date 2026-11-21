#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include "../src/midi/MidiFile.h"
#include "../src/midi/MidiChannel.h"
#include "../src/midi/MidiTrack.h"
#include "../src/MidiEvent/NoteOnEvent.h"
#include "../src/support/ProjectDocument.h"
#include <cassert>
#include "../src/midi/TempoClipboard.h"
static QByteArray read(const QString &p){QFile f(p);assert(f.open(QIODevice::ReadOnly));return f.readAll();}
int main(int argc,char**argv){
 QCoreApplication app(argc,argv);QTemporaryDir dir;assert(dir.isValid());
 const auto source=dir.filePath("original.mid"), project=dir.filePath("song.xms"), exported=dir.filePath("export.mid");
 MidiFile first;first.channel(0)->insertNote(60,0,96,100,first.track(0));
 {QFile f(source);assert(f.open(QIODevice::WriteOnly));f.write(first.serializeSourceMidi());}
 const auto original=read(source);bool ok=false;MidiFile imported(source,&ok);assert(ok);
 QJsonObject region{{"id",1},{"track",0},{"startTick",192},{"sourceStartTick",0},{"sourceEndTick",192},{"startNoteMode",2},{"endNoteMode",0},{"loopLengthTicks",384},{"quantizeGridTicks",48},{"quantizeStrength",100},{"quantizeSwing",50},{"program",27},{"name","Test Phrase"},{"ownedNotes",QJsonArray{QJsonArray{1,0,96,64,91,0,27}}}};
 QJsonObject state{{"format","XIV Midi Studio regions"},{"version",3},{"tracks",1},{"regions",QJsonArray{region}},{"managedTracks",QJsonArray{0}},{"sections",QJsonArray{QJsonArray{"Verse",0,768}}},{"assignedChannels",QJsonObject{{"0",-1}}}};
 xiv::MidiRegionModel model;assert(xiv::decodeProjectRegions(state,1,model));
 imported.setRegionLayout(model);imported.setProjectRegions(state);imported.track(0)->assignChannel(-1);
 imported.track(0)->setMuted(true);imported.track(0)->setHidden(true);
 imported.setExportMetadata(QJsonObject{{"title","April in Paris"},{"artist","Count Basie"},{"players",8}});
 assert(imported.saveProject(project));assert(read(source)==original);
 assert(!imported.save(source));assert(!imported.saveArrangedMidi(source));assert(!imported.saveArrangedMidi(exported));
 assert(QFile::remove(source));
 const auto legacy=dir.filePath("legacy.xivms");assert(QFile::copy(project,legacy));
 MidiFile legacyOpened(legacy,&ok);assert(ok && legacyOpened.isProject());
 MidiFile reopened(project,&ok);assert(ok && reopened.isProject());
 assert(reopened.track(0)->assignedChannel()==-1 && reopened.track(0)->muted() && reopened.track(0)->hidden());
 assert(reopened.projectRegions()==state);
 const auto originalTempo=xiv::readTempoMap(&reopened);
 auto tempo=originalTempo;
 xiv::TempoMap copied; copied[0]={500000,true,nullptr,{.25,.75}}; copied[96]={400000};
 const auto tempoBytes=xiv::encodeTempoClipboard(copied,{0,96},reopened.ticksPerQuarter());
 assert(xiv::pasteTempoClipboard(tempoBytes,reopened.ticksPerQuarter(),96,reopened.track(0),tempo).size()==2);
 xiv::writeTempoMap(&reopened,tempo,"Paste Tempo Points");
 assert(xiv::readTempoMap(&reopened).contains(192));
 reopened.protocol()->undo();
 assert(xiv::readTempoMap(&reopened).keys()==originalTempo.keys());
 reopened.protocol()->redo();
 assert(xiv::readTempoMap(&reopened)[96].rampToNext);
 reopened.protocol()->undo();
 assert(reopened.exportMetadata()==imported.exportMetadata());
 reopened.preparePlayerData(0);
 for(auto *e:*reopened.playerData()) assert(!dynamic_cast<NoteOnEvent*>(e));
 reopened.track(0)->assignChannel(1);reopened.track(0)->setMuted(false);
 assert(model.reassignOwnedNoteChannels({{0,1}}));reopened.setRegionLayout(model);
 assert(reopened.saveArrangedMidi(exported));
 MidiFile rendered(exported,&ok);assert(ok);
 int notes=0;for(auto *e:*rendered.channelEvents(1))if(auto*n=dynamic_cast<NoteOnEvent*>(e)){assert(n->note()==64);++notes;}
 assert(notes==2);
 assert(xiv::insideInstallFolder(dir.filePath("app/sub/song.mid"),dir.filePath("app")));
 assert(!xiv::insideInstallFolder(dir.filePath("app-other/song.mid"),dir.filePath("app")));
 QJsonObject broken=state;broken["tracks"]=2;assert(!xiv::decodeProjectRegions(broken,1,model));
 auto before=read(project);reopened.setProjectRegions(broken);assert(!reopened.saveProject(project));assert(read(project)==before);
}

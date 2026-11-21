#include "../src/midi/PerformanceVoiceEstimate.h"
#include "../src/midi/PreviewAudio.h"
#include "../src/support/VoiceWarning.h"
#include <cassert>
#include <algorithm>
#include <cstring>
#include <iostream>
int main(int argc,char**argv) {
 assert(argc==2);
 auto *settings=new_fluid_settings();
 fluid_settings_setnum(settings,"synth.sample-rate",44100);
 fluid_settings_setint(settings,"synth.reverb.active",0);
 fluid_settings_setint(settings,"synth.chorus.active",0);
 auto *synth=new_fluid_synth(settings);
 assert(fluid_synth_sfload(synth,argv[1],1)>=0);
 float left[512],right[512];
 float volume=1;
 auto render=[&]() { std::fill_n(left,512,0);std::fill_n(right,512,0);float *out[]={left,right};assert(xiv::renderPreviewAudio(synth,512,0,nullptr,2,out,volume)==FLUID_OK); };
 std::set<unsigned int> excluded;
 assert(xiv::estimatedVoiceCount(synth,excluded)==0);
 assert(fluid_synth_program_change(synth,0,0)==FLUID_OK);
 for(int key=48;key<64;++key) assert(fluid_synth_noteon(synth,0,key,100)==FLUID_OK);
 render();
 assert(xiv::estimatedVoiceCount(synth,excluded)==16);
 assert(std::any_of(left,left+512,[](float value){return value!=0;}));
 // Master volume does not alter the estimated note load.
 volume=0; render();
 for(float sample:left) assert(sample==0); for(float sample:right) assert(sample==0);
 assert(xiv::estimatedVoiceCount(synth,excluded)==16);
 for(int key=48;key<64;++key) fluid_synth_noteoff(synth,0,key);
 render();
 const int tails=xiv::estimatedVoiceCount(synth,excluded);
 assert(tails>0); // Released notes still use voices.
 for(int i=0;i<1800;++i) render();
 assert(xiv::estimatedVoiceCount(synth,excluded)==0);
 fluid_synth_noteon(synth,0,60,100); render();
 const auto before=xiv::soundingNoteIds(synth);
 fluid_synth_noteon(synth,0,67,100); render();
 for(auto id:xiv::soundingNoteIds(synth)) if(!before.count(id)) excluded.insert(id);
 assert(xiv::estimatedVoiceCount(synth,excluded)==1);
 fluid_synth_all_sounds_off(synth,0); render();
 assert(xiv::estimatedVoiceCount(synth,excluded)==0 && excluded.empty());
 assert(std::strcmp(xiv::voiceWarningColor(11),"#d8eaff")==0);
 assert(std::strcmp(xiv::voiceWarningColor(12),"#ffda65")==0);
 assert(std::strcmp(xiv::voiceWarningColor(15),"#ffda65")==0);
 assert(std::strcmp(xiv::voiceWarningColor(16),"#ff6464")==0);
 assert(std::strcmp(xiv::voiceWarningColor(30),"#ff6464")==0);
 delete_fluid_synth(synth);delete_fluid_settings(settings);
 std::cout<<"Voice estimate: tails, gain independence, excluded clicks, reset, thresholds PASS\n";
}

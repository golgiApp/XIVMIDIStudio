#pragma once
#include <fluidsynth.h>
namespace xiv {
// Apply the master fader AFTER synthesis. A zero synth gain can kill voices,
// destroying release tails and making a muted voice estimate misleading.
inline int renderPreviewAudio(fluid_synth_t *synth, int len, int nfx, float *fx[],
                              int nout, float *out[], float volume) {
    int result;
    if(nfx==0 && nout>=2) {
        float *effects[4]={out[0],out[1],out[0],out[1]};
        result=fluid_synth_process(synth,len,4,effects,nout,out);
    } else result=fluid_synth_process(synth,len,nfx,fx,nout,out);
    for(int channel=0;channel<nout;++channel)
        for(int i=0;i<len;++i) out[channel][i]*=volume;
    for(int channel=0;channel<nfx;++channel)
        for(int i=0;i<len;++i) fx[channel][i]*=volume;
    return result;
}
}

#include "../src/midi/TempoClipboard.h"
#include <cassert>
#include <iostream>
int main() {
 xiv::TempoMap source;
 source[100]={500000,true,nullptr,{.25,.75}};
 source[580]={400000,false,nullptr,{}};
 source[1000]={600000,false,nullptr,{}};
 const auto bytes=xiv::encodeTempoClipboard(source,{100,580},480);
 xiv::TempoMap dest; dest[0]={600000}; dest[2000]={700000}; dest[5000]={800000};
 auto selected=xiv::pasteTempoClipboard(bytes,960,1920,nullptr,dest);
 assert((selected==std::set<int>{1920,2880}));
 assert(dest.contains(0) && dest.contains(5000) && !dest.contains(2000));
 assert(dest[1920].rampToNext && dest[1920].curve.time==.25 && dest[1920].curve.progress==.75);
 assert(!dest[2880].rampToNext && dest[2880].microseconds==400000);
 const auto before=dest.keys();
 assert(xiv::pasteTempoClipboard(bytes,960,std::numeric_limits<int>::max(),nullptr,dest).empty());
 assert(dest.keys()==before);
 const auto one=xiv::encodeTempoClipboard(source,{100},480);
 assert(!QJsonDocument::fromJson(one).object()["points"].toArray()[0].toObject()["ramp"].toBool());
 std::cout<<"Tempo clipboard: PPQ scaling, ramp shape, segment replacement, atomic invalid paste PASS\n";
}

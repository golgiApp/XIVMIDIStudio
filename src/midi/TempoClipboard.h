#pragma once
#include "TempoMapEdit.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>
#include <set>
namespace xiv {
inline QByteArray encodeTempoClipboard(const TempoMap &points, const std::set<int> &selected, int tpq) {
    QJsonArray values;
    if(selected.empty()) return {};
    const int first=*selected.begin();
    for(int tick:selected) {
        auto it=points.find(tick); if(it==points.end()) continue;
        const auto &p=it.value(); const auto next=std::next(it);
        values.append(QJsonObject{{"tick",tick-first},{"us",p.microseconds},
            {"ramp",p.rampToNext && next!=points.end() && selected.count(next.key())!=0},
            {"time",p.curve.time},{"progress",p.curve.progress}});
    }
    return QJsonDocument(QJsonObject{{"version",1},{"tpq",tpq},{"points",values}}).toJson(QJsonDocument::Compact);
}
// Paste replaces only the covered tempo segment; external endpoints survive.
// This prevents pre-existing interior points from hijacking copied ramp targets.
inline std::set<int> pasteTempoClipboard(const QByteArray &bytes, int tpq, int playhead,
                                        MidiTrack *track, TempoMap &destination) {
    if(bytes.isEmpty() || bytes.size()>1024*1024 || tpq<=0 || playhead<0) return {};
    const auto root=QJsonDocument::fromJson(bytes).object();
    const int sourceTpq=root["tpq"].toInt();
    if(root["version"].toInt()!=1 || sourceTpq<=0 || sourceTpq>96000) return {};
    TempoMap incoming; std::set<int> selected;
    for(const auto &value:root["points"].toArray()) {
        const auto o=value.toObject(); const int offset=o["tick"].toInt(-1), us=o["us"].toInt();
        const double at=playhead+double(offset)*tpq/sourceTpq;
        if(offset<0 || at>std::numeric_limits<int>::max() || us<60060 || us>15000000) return {};
        TempoPoint p; p.microseconds=us; p.rampToNext=o["ramp"].toBool(); p.track=track;
        p.curve={o["time"].toDouble(.5),o["progress"].toDouble(.5)};
        if(!std::isfinite(p.curve.time) || !std::isfinite(p.curve.progress) ||
           p.curve.time<0 || p.curve.time>1 || p.curve.progress<0 || p.curve.progress>1) return {};
        const int tick=int(std::llround(at));
        if(incoming.contains(tick)) return {}; // Resolution too low; never silently lose points.
        incoming[tick]=p; selected.insert(tick);
    }
    if(selected.empty()) return {};
    incoming.last().rampToNext=false;
    auto updated=destination;
    auto it=updated.lowerBound(*selected.begin());
    while(it!=updated.end() && it.key()<=*selected.rbegin()) it=updated.erase(it);
    for(auto p=incoming.begin();p!=incoming.end();++p) updated[p.key()]=p.value();
    destination=std::move(updated); return selected;
}
}

#pragma once
#include "../region/MidiRegionTransfer.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <cmath>

namespace xiv {
inline constexpr auto regionMime = "application/x-xivms-regions-v1";
// Preserve independent note/region/tempo payloads when updating one clipboard view.
inline void setClipboardPayload(const QString &format, const QByteArray &bytes) {
    auto *next = new QMimeData;
    if (auto *old = QApplication::clipboard()->mimeData())
        for (const auto &name : old->formats())
            if(name==regionMime || name=="application/x-midieditor-events-v2" || name=="application/x-xivms-tempo-v1")
                next->setData(name, old->data(name));
    next->setData(format, bytes);
    QApplication::clipboard()->setMimeData(next);
}
inline QByteArray encodeRegionClipboard(const std::vector<RegionClipboardItem> &items, int tpq) {
    QJsonArray regions;
    for (const auto &item : items) {
        QJsonArray notes;
        for (const auto &n : item.clip.notes)
            notes.append(QJsonArray{n.id,n.startTick,n.endTick,n.pitch,n.velocity,n.channel,n.program});
        regions.append(QJsonObject{{"track",item.trackOffset},{"tick",item.tickOffset},
            {"duration",item.clip.duration},{"name",QString::fromStdString(item.clip.name)},
            {"program",item.clip.program},{"notes",notes}});
    }
    return QJsonDocument(QJsonObject{{"version",1},{"tpq",tpq},{"regions",regions}}).toJson(QJsonDocument::Compact);
}
inline std::vector<RegionClipboardItem> decodeRegionClipboard(const QByteArray &data, int targetTpq) {
    if (data.isEmpty() || data.size()>32*1024*1024 || targetTpq<=0) return {};
    const auto root=QJsonDocument::fromJson(data).object();
    const int tpq=root["tpq"].toInt();
    if(root["version"].toInt()!=1 || tpq<=0 || tpq>96000) return {};
    auto scale=[&](int n)->int {
        const double value=double(n)*targetTpq/tpq;
        return n<0 || value>std::numeric_limits<int>::max() ? -1 : int(std::llround(value));
    };
    std::vector<RegionClipboardItem> result;
    size_t count=0;
    for(const auto &value:root["regions"].toArray()) {
        auto o=value.toObject(); RegionClipboardItem item;
        item.trackOffset=o["track"].toInt(-1); item.tickOffset=scale(o["tick"].toInt(-1));
        item.clip.duration=scale(o["duration"].toInt(-1));
        item.clip.name=o["name"].toString().toStdString(); item.clip.program=o["program"].toInt(-1);
        if(item.trackOffset<0 || item.trackOffset>65535 || item.tickOffset<0 || item.clip.duration<=0 || item.clip.program < -1 || item.clip.program>127) return {};
        for(const auto &entry:o["notes"].toArray()) {
            auto a=entry.toArray(); if(a.size()!=7 || ++count>250000) return {};
            OwnedRegionNote n{a[0].toInt(),scale(a[1].toInt(-1)),scale(a[2].toInt(-1)),a[3].toInt(-1),a[4].toInt(-1),a[5].toInt(-1),a[6].toInt(-1)};
            if(n.endTick==n.startTick && n.startTick>=0 && n.endTick<item.clip.duration) ++n.endTick;
            if(n.endTick>item.clip.duration) return {};
            item.clip.notes.push_back(n);
        }
        if(!validOwnedNotes(item.clip.notes)) return {};
        result.push_back(std::move(item));
        if(result.size()>65535) return {};
    }
    return result;
}
inline std::vector<RegionClipboardItem> systemRegionClipboard(int tpq) {
    const auto *mime=QApplication::clipboard()->mimeData();
    return mime ? decodeRegionClipboard(mime->data(regionMime),tpq) : std::vector<RegionClipboardItem>{};
}
}

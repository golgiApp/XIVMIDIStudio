#pragma once
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include "../region/MidiRegionModel.h"

namespace xiv {
// Resolve existing parents as well as existing files (junction/symlink aliases).
inline QString resolvedUserPath(const QString &path) {
    QFileInfo info(QDir::cleanPath(QFileInfo(path).absoluteFilePath()));
    if (info.exists()) return info.canonicalFilePath();
    QStringList suffix;
    while (!info.exists()) {
        suffix.prepend(info.fileName());
        const QString parent = info.absolutePath();
        if (parent == info.absoluteFilePath()) break;
        info.setFile(parent);
    }
    QString result = info.canonicalFilePath();
    if (result.isEmpty()) result = info.absoluteFilePath();
    for (const auto &part : suffix) result = QDir(result).filePath(part);
    return QDir::cleanPath(result);
}
inline bool sameUserPath(const QString &a, const QString &b) {
    if (a.isEmpty() || b.isEmpty()) return false;
#ifdef Q_OS_WIN
    return resolvedUserPath(a).compare(resolvedUserPath(b), Qt::CaseInsensitive) == 0;
#else
    return resolvedUserPath(a) == resolvedUserPath(b);
#endif
}
inline bool insideInstallFolder(const QString &path, const QString &installation = QCoreApplication::applicationDirPath()) {
    const QString target = resolvedUserPath(path), root = resolvedUserPath(installation);
#ifdef Q_OS_WIN
    constexpr auto sensitivity = Qt::CaseInsensitive;
#else
    constexpr auto sensitivity = Qt::CaseSensitive;
#endif
    return target.compare(root, sensitivity) == 0 || target.startsWith(root + '/', sensitivity);
}
inline bool decodeProjectRegions(const QJsonObject &root, int tracks, MidiRegionModel &model) {
    if (root.value("format").toString() != "XIV Midi Studio regions" ||
        root.value("tracks").toInt(-1) != tracks || !root.value("regions").isArray()) return false;
    const int version = root.value("version").toInt();
    if (version < 1 || version > 3) return false;
    const auto assignments = root.value("assignedChannels").toObject();
    for (auto it = assignments.begin(); it != assignments.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok), channel = it.value().toInt(-2);
        if (!ok || index < 0 || index >= tracks || channel < -1 || channel > 15) return false;
    }
    std::vector<MidiRegion> regions;
    for (const auto &v : root.value("regions").toArray()) {
        if (!v.isObject()) return false;
        const auto o=v.toObject();
        MidiRegion r{o.value("id").toInt(-1),o.value("track").toInt(-1),o.value("startTick").toInt(-1),
                     o.value("sourceStartTick").toInt(-1),o.value("sourceEndTick").toInt(-1)};
        r.startNoteMode=static_cast<CrossingNoteMode>(o.value("startNoteMode").toInt());
        r.endNoteMode=static_cast<CrossingNoteMode>(o.value("endNoteMode").toInt());
        r.loopLengthTicks=o.value("loopLengthTicks").toInt();
        r.quantize={o.value("quantizeGridTicks").toInt(),o.value("quantizeStrength").toInt(100),o.value("quantizeSwing").toInt(50)};
        r.program=o.value("program").toInt(-1);
        r.name=o.value("name").toString().toStdString();
        if (o.contains("ownedNotes")) {
            if (!o.value("ownedNotes").isArray()) return false;
            std::vector<OwnedRegionNote> notes;
            for (const auto &n : o.value("ownedNotes").toArray()) {
                const auto a=n.toArray(); if(a.size()!=6 && a.size()!=7) return false;
                notes.push_back({a[0].toInt(-1),a[1].toInt(-1),a[2].toInt(-1),a[3].toInt(-1),a[4].toInt(-1),a[5].toInt(-1),a.size()==7?a[6].toInt(-1):-1});
            }
            r.ownedNotes=std::make_shared<const std::vector<OwnedRegionNote>>(std::move(notes));
        }
        regions.push_back(std::move(r));
    }
    MidiRegionModel result;
    for(const auto &v:root.value("managedTracks").toArray()) {
        int t=v.toInt(-1);if(t<0 || t>=tracks)return false;result.markManagedTrack(t);
    }
    if(!result.replaceValidated(regions,tracks))return false;
    model=std::move(result);return true;
}
}

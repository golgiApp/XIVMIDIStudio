#pragma once
#include <QObject>
#include <QString>
#include <vector>
#include <utility>

inline std::vector<std::pair<QString, std::vector<int>>> xivInstrumentGroups() {
    return {
                    {QObject::tr("Woodwinds"), {72, 73, 68, 71, 65, 75}},
                    {QObject::tr("Brass"), {56, 60, 57, 58}},
                    {QObject::tr("Percussion"), {47, 96, 98, 97, 99}},
                    {QObject::tr("Orchestral Strings"), {40, 41, 42, 43}},
                    {QObject::tr("Piano & Plucked Strings"), {0, 46, 25, 45}},
                    {QObject::tr("Electric Guitars"), {29, 27, 28, 30, 31}}
                };
}

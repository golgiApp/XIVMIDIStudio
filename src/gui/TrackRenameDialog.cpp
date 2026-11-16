#include <algorithm>
#include <QMap>
#include "../support/XivInstrumentGroups.h"
/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TrackRenameDialog.h"
#include "Appearance.h"
#include "GameSupportSettingsWidget.h"
#include "../support/FFXIVChannelFixer.h"
#include "../support/SoundFontPresetCatalog.h"
#include <QFileInfo>
#include <QSet>
#include "../midi/InstrumentDefinitions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QSettings>

TrackRenameDialog::TrackRenameDialog(const QString &currentName, int trackNumber, QSettings *settings, QWidget *parent,
                                     const QStringList &soundFonts, bool tutorial)
    : QDialog(parent), _settings(settings), _trackNumber(trackNumber) {
    
    setWindowTitle(tr("Set Track Name"));
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 10, 20, 20);
    mainLayout->setSpacing(5);

    QHBoxLayout *topLayout = new QHBoxLayout();
    QLabel *label = new QLabel(tr("Track Name (Track %1)").arg(trackNumber), this);
    topLayout->addWidget(label);
    topLayout->addStretch();

    if (!soundFonts.isEmpty() || tutorial || GameSupportSettingsWidget::isFFXIVRenamePresetsEnabled(_settings)) {
        _presetsButton = new QToolButton(this);
        _presetsButton->setIcon(QIcon(":/run_environment/graphics/channelwidget/instrument.png"));
        _presetsButton->setIconSize(QSize(20, 20));
        _presetsButton->setFixedSize(QSize(30, 30));
        _presetsButton->setToolTip(soundFonts.isEmpty() ? tr("FFXIV Instrument Presets")
                                                         : tr("SoundFont Presets"));
        _presetsButton->setPopupMode(QToolButton::InstantPopup);
        
        QMenu *menu = new QMenu(_presetsButton);
        menu->setStyleSheet("QMenu { menu-scrollable: 1; }"); // Ensure scrolling if too many items

        if (!soundFonts.isEmpty()) {
            QSet<int> seen;
            for (const auto &path : soundFonts) {
                auto *font = soundFonts.size() == 1 ? menu : menu->addMenu(QFileInfo(path).completeBaseName());
                QMap<int, QMenu *> families;
                if (QFileInfo(path).completeBaseName().compare(QStringLiteral("FFXIV Standard"), Qt::CaseInsensitive) == 0) {
                    for (const auto &group : xivInstrumentGroups()) {
                        auto *family = font->addMenu(group.first);
                        for (int program : group.second) families.insert(program, family);
                    }
                }
                auto presets = xivReadSoundFontPresets(path);
                QList<int> order;
                for (const auto &group : xivInstrumentGroups())
                    for (int program : group.second) order.append(program);
                if (!families.isEmpty())
                    std::stable_sort(presets.begin(), presets.end(), [&order](const auto &a, const auto &b) {
                        const int ai = order.indexOf(a.program), bi = order.indexOf(b.program);
                        return (ai < 0 ? 999 : ai) < (bi < 0 ? 999 : bi);
                    });
                for (const auto &preset : presets) {
                    if (preset.bank != 0 || seen.contains(preset.program)) continue;
                    seen.insert(preset.program);
                    auto *action = families.value(preset.program, font)->addAction(preset.name);
                    connect(action, &QAction::triggered, this, [this, preset] {
                        _program = preset.program;
                        _nameEdit->setText(preset.name);
                        _nameEdit->setFocus();
                    });
                }
                if (font->isEmpty()) font->menuAction()->setVisible(false);
            }
            if (tutorial && !seen.contains(58)) {
                auto *brass = menu->addMenu(tr("Brass"));
                auto *tuba = brass->addAction(tr("Tuba"));
                tuba->setData(tr("Tuba"));
                tuba->setProperty("program", 58);
                connect(tuba, &QAction::triggered, this, &TrackRenameDialog::setPresetName);
            }
        } else {
            QList<FFXIVChannelFixer::InstrumentGroup> groups = FFXIVChannelFixer::ffxivInstrumentGroups();
            InstrumentDefinitions *defs = InstrumentDefinitions::instance();

            for (const auto &group : groups) {
                QMenu *groupMenu = menu->addMenu(group.name);
                for (int program : group.programs) {
                    QString name = defs->instrumentName(program);
                    name.remove(" ");
                    if (group.name == tr("Guitars")) {
                        name.remove(":");
                    }

                    QAction *action = groupMenu->addAction(name);
                    action->setData(name);
                    action->setProperty("program", program);
                    connect(action, SIGNAL(triggered()), this, SLOT(setPresetName()));
                }

                if (group.name == tr("Guitars")) {
                    QString special = "Program:ElectricGuitar";
                    QAction *action = groupMenu->addAction(special);
                    action->setData(special);
                    connect(action, SIGNAL(triggered()), this, SLOT(setPresetName()));
                }
            }
        }

        _presetsButton->setMenu(menu);
        topLayout->addWidget(_presetsButton);
    }

    mainLayout->addLayout(topLayout);

    _nameEdit = new QLineEdit(currentName, this);
    _nameEdit->setPlaceholderText(tr("Enter track name..."));
    _nameEdit->setFixedHeight(30);
    _nameEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { color:#F4F8FF; background:#26313D; border:1px solid #526780; "
        "border-radius:4px; padding:3px 6px; selection-background-color:#316B9E; "
        "selection-color:#FFFFFF; } QLineEdit:focus { border-color:#71B4EA; }"));
    _nameEdit->selectAll();
    mainLayout->addWidget(_nameEdit);

    mainLayout->addSpacing(10);
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(cancelButton);

    buttonLayout->addStretch();

    QPushButton *acceptButton = new QPushButton(tr("Accept"), this);
    acceptButton->setDefault(true);
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(acceptButton);

    mainLayout->addLayout(buttonLayout);
}

QString TrackRenameDialog::name() const {
    return _nameEdit->text();
}

void TrackRenameDialog::showPresets() {
    // Handled by QToolButton popup mode
}

void TrackRenameDialog::setPresetName() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        if (action->property("program").isValid()) _program = action->property("program").toInt();
        else _program = -1;
        _nameEdit->setText(action->data().toString());
        _nameEdit->setFocus();
    }
}

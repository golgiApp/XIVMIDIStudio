#pragma once
#include <QString>
#include <QRegularExpression>
namespace xiv {
inline QString exportBaseName(QString pattern, const QString &title, const QString &artist,
                              const QString &editor, int players) {
    pattern.replace("{title}",title).replace("{artist}",artist).replace("{editor}",editor)
        .replace("{players}",players>0?QString::number(players):QString());
    pattern.replace(QRegularExpression(QStringLiteral("[<>:\"/\\\\|?*\\x00-\\x1f]")),"_");
    pattern=pattern.simplified();
    while(pattern.endsWith('.') || pattern.endsWith(' ')) pattern.chop(1);
    if(pattern.endsWith(".mid",Qt::CaseInsensitive)) pattern.chop(4);
    pattern=pattern.left(180);
    while(pattern.endsWith('.') || pattern.endsWith(' ')) pattern.chop(1);
    if(pattern.isEmpty()) pattern="Untitled";
    if(QRegularExpression("^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])($|\\.)",QRegularExpression::CaseInsensitiveOption).match(pattern).hasMatch()) pattern.prepend('_');
    return pattern.left(180);
}
}

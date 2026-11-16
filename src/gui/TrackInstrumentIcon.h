#ifndef XIV_TRACK_INSTRUMENT_ICON_H
#define XIV_TRACK_INSTRUMENT_ICON_H

#include <QPainter>
#include <QPainterPath>

namespace xiv {

// Draw the instrument at the origin in a 36-by-36 logical-pixel box.
// Program numbers are zero-based and follow the XIV SoundFont assignments.
inline void drawTrackInstrumentIcon(QPainter &p, int program) {
    const QColor ink = p.pen().color();
    const QPen outline(ink, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(outline);
    p.setBrush(Qt::NoBrush);
    QPainterPath shape;

    switch (program) {
    case 0: case 1: case 2: case 3:
        shape.moveTo(-16, 4);
        shape.lineTo(7, -13);
        shape.quadTo(13, -15, 11, -8);
        shape.quadTo(10, -5, 15, -3);
        shape.quadTo(17, -2, 17, 3);
        shape.lineTo(-16, 4);
        p.fillPath(shape, ink);
        p.drawRoundedRect(QRectF(-16, 5, 33, 4), 1, 1);
        p.drawLine(QPointF(-12, 9), QPointF(-12, 14));
        p.drawLine(QPointF(13, 9), QPointF(13, 14));
        break;
    case 24: case 25: case 26:
    case 40: case 41: case 42: case 43: case 45: case 110: {
        const bool bowed = program >= 40;
        p.rotate(38);
        shape.moveTo(-2, -4);
        shape.cubicTo(-9, -7, -10, 0, -7, 3);
        shape.cubicTo(-6, 5, -12, 6, -10, 12);
        shape.cubicTo(-8, 19, 8, 19, 10, 12);
        shape.cubicTo(12, 6, 6, 5, 7, 3);
        shape.cubicTo(10, 0, 9, -7, 2, -4);
        shape.closeSubpath();
        p.drawPath(shape);
        p.drawLine(QPointF(0, 3), QPointF(0, -16));
        p.drawRoundedRect(QRectF(-2.5, -18, 5, 5), 1, 1);
        if (bowed) {
            p.drawLine(QPointF(-4, 7), QPointF(4, 7));
            p.drawLine(QPointF(12, -15), QPointF(12, 11));
        } else {
            p.drawEllipse(QPointF(0, 7), 2.5, 2.5);
        }
        break;
    }
    case 27: case 28: case 29: case 30: case 31: {
        p.save();
        p.translate(-4, -1);
        p.rotate(41);
        p.scale(.88, .88);
        shape.moveTo(-2, -4);
        shape.cubicTo(-5, -4, -6, -7, -7, -9);
        shape.cubicTo(-11, -9, -11, -2, -8, 2);
        shape.cubicTo(-8, 6, -12, 10, -9, 14);
        shape.cubicTo(-5, 19, 8, 18, 10, 13);
        shape.cubicTo(12, 9, 8, 6, 8, 2);
        shape.cubicTo(8, -2, 9, -5, 6, -5);
        shape.quadTo(6, 0, 2, -2);
        p.drawPath(shape);
        p.drawPolyline(QPolygonF{{-2, 3}, {-2, -15}, {-3, -16}, {-2, -20},
                                 {3, -21}, {4, -17}, {2, -14}, {2, 3}});
        for (int y : {5, 8, 11}) p.drawLine(QPointF(-3, y), QPointF(3, y));
        p.drawEllipse(QPointF(6, 10), .85, .85);
        p.restore();
        const char *numerals[] = {"II", "III", "I", "IV", "V"};
        QFont numeralFont(QStringLiteral("serif"));
        numeralFont.setPixelSize(13);
        numeralFont.setBold(true);
        p.setFont(numeralFont);
        p.drawText(QRectF(0, 0, 20, 16), Qt::AlignRight | Qt::AlignVCenter,
                   QString::fromLatin1(numerals[program - 27]));
        break;
    }
    case 46:
        // Curved neck, tall pillar and diagonal soundbox of a concert harp.
        shape.moveTo(-15, 13);
        shape.lineTo(-15, -11);
        shape.quadTo(-14, -17, -8, -16);
        shape.cubicTo(-3, -16, 0, -7, 6, -7);
        shape.quadTo(10, -7, 14, -11);
        shape.lineTo(16, -8);
        shape.lineTo(-9, 15);
        shape.lineTo(-15, 15);
        shape.closeSubpath();
        p.drawPath(shape);
        p.drawLine(QPointF(-11, -12), QPointF(-11, 10));
        for (const QLineF &string : {QLineF(-7,-12,-7,9), QLineF(-3,-8,-3,6),
                                    QLineF(1,-5,1,2), QLineF(5,-4,5,-2)})
            p.drawLine(string);
        p.drawLine(QPointF(-17, 16), QPointF(-7, 16));
        break;
    case 56: {
        p.rotate(-15);
        p.drawLine(QPointF(-17, 0), QPointF(8, 0));
        p.drawLine(QPointF(-17, -2), QPointF(-17, 2));
        shape.moveTo(-12, 0);
        shape.cubicTo(-15, 6, -11, 8, -7, 8);
        shape.lineTo(2, 8);
        shape.quadTo(9, 8, 7, 1);
        p.drawPath(shape);
        for (int x : {-7, -3, 1}) {
            p.drawLine(QPointF(x, -5), QPointF(x, 5));
            p.drawLine(QPointF(x - 1, -5), QPointF(x + 1, -5));
        }
        QPainterPath bell;
        bell.moveTo(4, -1);
        bell.quadTo(11, -2, 13, -8);
        bell.lineTo(13, 8);
        bell.quadTo(11, 2, 4, 1);
        p.fillPath(bell, ink);
        p.drawEllipse(QRectF(12, -8, 5, 16));
        break;
    }
    case 57: {
        p.setPen(QPen(ink, 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        // Trace the rear bow, widening bell throat, and extended slide separately.
        shape.moveTo(-1.2, -6.9);
        shape.lineTo(-17.2, -11.2);
        shape.cubicTo(-20, -12, -19.5, -6.4, -17.2, -5.5);
        shape.lineTo(-6.3, -1.6);
        shape.moveTo(-14.8, -10.5);
        shape.lineTo(-14.2, -4.5);
        shape.moveTo(-8.2, -8.6);
        shape.lineTo(-7.7, -2.1);
        shape.moveTo(-15.9, -10.8);
        shape.cubicTo(-10.3, -9.1, -4.6, -7.4, -2, -4.5);
        p.drawPath(shape);

        p.save();
        p.translate(1.8, -4.3);
        p.rotate(17);
        p.drawEllipse(QRectF(-3.3, -5.3, 6.6, 10.6));
        p.setPen(QPen(ink, .8));
        p.drawArc(QRectF(-2.8, -3.7, 4.3, 7.4), 75 * 16, 220 * 16);
        p.restore();

        shape = QPainterPath();
        shape.moveTo(-6.3, -1.6);
        shape.lineTo(17, 8.3);
        shape.cubicTo(20.5, 9.7, 19.2, 12, 15.3, 10.8);
        shape.lineTo(-11.8, -.6);
        shape.moveTo(-9.9, .2);
        shape.lineTo(-5.8, -1.4);
        p.drawPath(shape);
        p.drawLine(QPointF(-12.5, -1.3), QPointF(-13, -.2));
        break;
    }
    case 58:
        p.drawEllipse(QRectF(-3, -16, 18, 5));
        shape.moveTo(-2, -12);
        shape.quadTo(4, -7, 4, 4);
        shape.quadTo(4, 13, -3, 13);
        shape.quadTo(-10, 13, -10, 6);
        shape.lineTo(-10, -4);
        shape.quadTo(-10, -7, -6, -7);
        p.drawPath(shape);
        shape = QPainterPath();
        shape.moveTo(14, -12);
        shape.quadTo(8, -6, 8, 5);
        shape.cubicTo(8, 20, -15, 20, -15, 7);
        shape.lineTo(-15, -3);
        shape.quadTo(-15, -10, -7, -10);
        p.drawPath(shape);
        p.drawLine(QPointF(-6, -6), QPointF(-6, 5));
        p.drawLine(QPointF(-2, -5), QPointF(-2, 5));
        break;
    case 60: {
        // Bell and outer contour, with only the foreground valve tubing.
        p.setPen(QPen(ink, 1.35, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        shape.moveTo(-17, 5);
        shape.quadTo(-12, -3, -5.5, -9);
        shape.cubicTo(-9, -3, -6.5, 1.5, -3, 5.5);
        shape.moveTo(-17, 5);
        shape.cubicTo(-12, .9, -8.8, 3.1, -4, 7);
        shape.quadTo(0, 10.5, 5.8, 11);
        shape.moveTo(17, -4.3);
        shape.lineTo(17, -14.3);
        p.drawEllipse(QRectF(-4.2, -11, 22, 22));
        p.drawPath(shape);
        p.drawLine(QPointF(15.9, -14.3), QPointF(18.1, -14.3));

        p.save();
        p.translate(2.65, 0.55);
        shape = QPainterPath();
        shape.moveTo(10.7, -3);
        shape.lineTo(1, -6.4);
        shape.cubicTo(-1.4, -7.4, -.2, -10.1, 2, -9.7);
        shape.quadTo(3.7, -9.4, 2.6, -8.3);
        shape.moveTo(9.8, .1);
        shape.lineTo(2.2, -2.6);
        shape.quadTo(.2, -3.3, .2, -1.9);
        shape.quadTo(.2, -.8, 2.1, -.2);
        shape.lineTo(8.8, 2.4);
        shape.moveTo(8.8, 5.4);
        shape.lineTo(-.6, 2);
        shape.cubicTo(-2.3, 1.4, -2.8, 4.8, -.7, 6.8);
        shape.cubicTo(2.4, 9.3, 5.1, 8.7, 4.5, 6.8);
        p.drawPath(shape);
        for (const QPointF &valve : {QPointF(11.7, -2.7), QPointF(10.7, .5),
                                    QPointF(9.7, 3.6)})
            p.drawEllipse(valve, .8, .8);
        p.restore();
        break;
    }
    case 64: case 65: case 66: case 67: {
        // Profile traced from the alto reference, from the cork to the bow.
        p.setPen(QPen(ink, 1.35, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        shape.moveTo(-3, -17);
        shape.lineTo(3.5, -12.7);
        shape.cubicTo(5.2, -11.6, 5.3, -10.1, 4.7, -8.3);
        shape.lineTo(-1.6, 10.7);
        shape.cubicTo(-2.6, 13.9, -.1, 14.9, 1.6, 12.4);
        shape.cubicTo(3.1, 10.2, 3.7, 5.6, 5.8, 3.2);
        shape.quadTo(6.4, 2.4, 6.5, 1.8);
        shape.moveTo(-4.7, -14.6);
        shape.lineTo(1.7, -10.2);
        shape.quadTo(2.4, -9.6, 2, -8.5);
        shape.lineTo(-5.5, 12);
        shape.cubicTo(-7.7, 19, 2.1, 20.5, 5.2, 13.8);
        shape.cubicTo(7, 9.9, 6.6, 7.5, 10.1, 6);
        p.drawPath(shape);
        p.drawLine(QPointF(-4.7, -14.6), QPointF(-3, -17));
        p.drawLine(QPointF(-2.3, -13), QPointF(-.7, -15.4));
        p.save();
        p.translate(8.3, 3.9);
        p.rotate(49);
        p.drawEllipse(QRectF(-3.9, -1.25, 7.8, 2.5));
        p.restore();
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        for (const QPointF &key : {QPointF(2.1, -5.5), QPointF(1, -2.3),
                                   QPointF(-.1, .9), QPointF(-1.2, 4.1),
                                   QPointF(-2.3, 7.3), QPointF(-3.4, 10.5)})
            p.drawEllipse(key, .65, .65);
        break;
    }
    case 68: case 69: case 70: case 71: {
        const bool oboe = program == 68;
        p.rotate(44);
        shape.moveTo(-1.4, -11);
        shape.lineTo(1.4, -11);
        shape.lineTo(oboe ? 2.5 : 2.1, 9);
        shape.quadTo(2.5, 12, oboe ? 4 : 5, 15);
        shape.quadTo(0, 17, oboe ? -4 : -5, 15);
        shape.quadTo(-2.5, 12, oboe ? -2.5 : -2.1, 9);
        shape.closeSubpath();
        p.drawPath(shape);
        if (oboe) {
            p.drawLine(QPointF(0,-12), QPointF(0,-15));
            p.drawLine(QPointF(-1,-16), QPointF(1,-16));
        } else {
            p.drawPolyline(QPolygonF{{-1.4,-11}, {-1.4,-16}, {1,-14}, {1.4,-11}});
            p.drawLine(QPointF(-3, -7), QPointF(3, -7));
        }
        for (int y : {-3, 1, 5, 9})
            p.drawPoint(QPointF(0, y));
        break;
    }
    case 72: case 73: case 74:
        p.rotate(-38);
        p.drawRoundedRect(QRectF(-17, -2.2, 34, 4.4), 1, 1);
        for (int x : {-10, -5, 0, 5, 10}) p.drawEllipse(QPointF(x, -4), 1, 1);
        break;
    case 75:
        for (int i = 0; i < 6; ++i)
            p.drawRoundedRect(QRectF(-15 + 5 * i, -11, 4, 25 - 3 * i), 1, 1);
        p.drawLine(QPointF(-16, -4), QPointF(15, -4));
        break;
    case 47: case 98:
        p.drawEllipse(QRectF(-14, -9, 28, 9));
        shape.moveTo(-14, -4);
        if (program == 47) {
            shape.quadTo(-14, 12, 0, 12);
            shape.quadTo(14, 12, 14, -4);
        } else {
            shape.lineTo(-14, 7);
            shape.quadTo(0, 16, 14, 7);
            shape.lineTo(14, -4);
            p.drawLine(QPointF(-9, -13), QPointF(5, -4));
            p.drawLine(QPointF(8, -13), QPointF(-3, -4));
        }
        p.drawPath(shape);
        p.drawLine(QPointF(-8, 0), QPointF(-6, 9));
        p.drawLine(QPointF(8, 0), QPointF(6, 9));
        break;
    case 96:
        p.drawEllipse(QRectF(-17, -8, 14, 6));
        p.drawEllipse(QRectF(0, -10, 17, 7));
        p.drawPolyline(QPolygonF{QPointF(-17, -5), QPointF(-14, 9), QPointF(-6, 9), QPointF(-3, -5)});
        p.drawPolyline(QPolygonF{QPointF(0, -6), QPointF(3, 12), QPointF(14, 12), QPointF(17, -6)});
        break;
    case 97:
        p.drawEllipse(QRectF(-13, -14, 26, 26));
        p.drawEllipse(QRectF(-9, -10, 18, 18));
        p.drawLine(QPointF(-9, 9), QPointF(-14, 15));
        p.drawLine(QPointF(9, 9), QPointF(14, 15));
        break;
    case 99:
        p.save();
        p.rotate(-18);
        shape.moveTo(-16, 4);
        shape.quadTo(-11, 0, -5, -1);
        shape.cubicTo(-4, -7, 3, -8, 6, -2);
        shape.quadTo(13, -1, 17, 3);
        shape.quadTo(0, 7, -16, 4);
        p.drawPath(shape);
        p.drawLine(QPointF(0, -9), QPointF(0, -6));
        p.restore();
        p.drawLine(QPointF(0, 5), QPointF(0, 15));
        p.drawLine(QPointF(-5, 16), QPointF(5, 16));
        break;
    default:
        p.setBrush(ink);
        p.drawEllipse(QRectF(-10, 3, 10, 7));
        p.drawLine(QPointF(0, 6), QPointF(0, -13));
        p.drawLine(QPointF(0, -13), QPointF(9, -8));
        break;
    }
}

} // namespace xiv

#endif

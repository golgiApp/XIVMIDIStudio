#ifndef XIV_WORKSPACE_CONTROL_ICONS_H
#define XIV_WORKSPACE_CONTROL_ICONS_H

#include <QIconEngine>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>

namespace xiv {
enum class ControlIcon { Height, Width, Volume, Muted, Eye, Hidden };

// Vector geometry avoids source-encoding and installed-font dependencies.
inline void paintControlIcon(QPainter &p, const QRect &rect, ControlIcon kind) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(rect.center());
    const qreal scale = qMin(rect.width(), rect.height()) / 24.0;
    p.scale(scale, scale);
    p.setPen(QPen(QColor("#F5F8FC"), 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    if (kind == ControlIcon::Height || kind == ControlIcon::Width) {
        if (kind == ControlIcon::Width) p.rotate(90);
        p.drawLine(QPointF(0, -8), QPointF(0, 8));
        p.drawPolyline(QPolygonF{QPointF(-4, -4), QPointF(0, -8), QPointF(4, -4)});
        p.drawPolyline(QPolygonF{QPointF(-4, 4), QPointF(0, 8), QPointF(4, 4)});
    } else if (kind == ControlIcon::Eye || kind == ControlIcon::Hidden) {
        QPainterPath eye;
        eye.moveTo(-9, 0); eye.quadTo(0, -11, 9, 0); eye.quadTo(0, 11, -9, 0);
        p.drawPath(eye); p.drawEllipse(QPointF(0, 0), 2.5, 2.5);
        if (kind == ControlIcon::Hidden) p.drawLine(QPointF(-8, 8), QPointF(8, -8));
    } else {
        p.drawPolygon(QPolygonF{QPointF(-9, -3), QPointF(-5, -3), QPointF(0, -7),
                              QPointF(0, 7), QPointF(-5, 3), QPointF(-9, 3)});
        if (kind == ControlIcon::Muted) {
            p.drawLine(QPointF(-10, 9), QPointF(10, -9));
        } else {
            QPainterPath waves;
            waves.moveTo(4, -4); waves.quadTo(8, 0, 4, 4);
            waves.moveTo(7, -7); waves.quadTo(14, 0, 7, 7);
            p.drawPath(waves);
        }
    }
    p.restore();
}

class ControlIconEngine final : public QIconEngine {
public:
    explicit ControlIconEngine(ControlIcon kind) : kind_(kind) {}
    QIconEngine *clone() const override { return new ControlIconEngine(kind_); }
    void paint(QPainter *p, const QRect &r, QIcon::Mode, QIcon::State) override {
        paintControlIcon(*p, r, kind_);
    }
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
        QPixmap result(size); result.fill(Qt::transparent);
        QPainter painter(&result); paint(&painter, QRect(QPoint(), size), mode, state);
        return result;
    }
private:
    ControlIcon kind_;
};
inline QIcon controlIcon(ControlIcon kind) { return QIcon(new ControlIconEngine(kind)); }

class ZoomIndicator final : public QWidget {
public:
    ZoomIndicator(bool vertical, QWidget *parent) : QWidget(parent),
        kind_(vertical ? ControlIcon::Height : ControlIcon::Width) {
        setFixedSize(24, 24);
        setAccessibleName(vertical ? tr("Track or note height") : tr("Timeline width"));
        setToolTip(accessibleName());
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this); paintControlIcon(painter, rect(), kind_);
    }
private:
    ControlIcon kind_;
};
} // namespace xiv
#endif

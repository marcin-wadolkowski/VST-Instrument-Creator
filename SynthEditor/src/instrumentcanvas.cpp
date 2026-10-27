#include "instrumentcanvas.h"

#include <QColor>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QtMath>
#include <QSize>

InstrumentCanvas::InstrumentCanvas(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(500, 300);
    setMouseTracking(true);
    setAcceptDrops(true);
}

void InstrumentCanvas::setElements(const QVector<InstrumentElement> &elements)
{
    m_elements = elements;
    update();
}

void InstrumentCanvas::setSelectedId(const QString &id)
{
    m_selectedId = id;
    update();
}

QString InstrumentCanvas::selectedId() const { return m_selectedId; }

void InstrumentCanvas::setCanvasBackground(const QString &color, const QString &imagePath)
{
    m_backgroundColor = color;
    m_backgroundImagePath = imagePath;
    m_backgroundImage = imagePath.isEmpty() ? QImage{} : QImage(imagePath);
    update();
}

void InstrumentCanvas::setCanvasLogicalSize(int width, int height)
{
    m_logicalWidth = qBound(200, width, 4000);
    m_logicalHeight = qBound(150, height, 3000);
    update();
}

QPoint InstrumentCanvas::canvasPoint(const QPoint &p) const
{
    const double sx = double(m_logicalWidth) / qMax(1, width());
    const double sy = double(m_logicalHeight) / qMax(1, height());
    return QPoint(qRound(p.x() * sx), qRound(p.y() * sy));
}

QRect InstrumentCanvas::elementRect(const InstrumentElement &e) const
{
    const double sx = double(width()) / qMax(1, m_logicalWidth);
    const double sy = double(height()) / qMax(1, m_logicalHeight);
    return QRect(qRound(e.x * sx), qRound(e.y * sy),
                 qRound(e.width * sx), qRound(e.height * sy));
}

int InstrumentCanvas::elementAt(const QPoint &point) const
{
    for (int i = m_elements.size() - 1; i >= 0; --i)
        if (elementRect(m_elements.at(i)).contains(point))
            return i;
    return -1;
}

bool InstrumentCanvas::onResizeHandle(const QRect &rect, const QPoint &point) const
{
    return QRect(rect.right() - 12, rect.bottom() - 12, 16, 16).contains(point);
}

void InstrumentCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(m_backgroundColor));

    if (!m_backgroundImage.isNull())
        p.drawImage(rect(), m_backgroundImage);

    // Grid is shown at the logical canvas scale.
    p.save();
    p.setPen(QPen(QColor(255, 255, 255, 15), 1));
    const int stepX = qMax(8, qRound(double(width()) * 20 / m_logicalWidth));
    const int stepY = qMax(8, qRound(double(height()) * 20 / m_logicalHeight));
    for (int x = 0; x < width(); x += stepX) p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += stepY) p.drawLine(0, y, width(), y);
    p.restore();

    for (const auto &e : m_elements) {
        QRect r = elementRect(e);
        const bool selected = e.id == m_selectedId;
        const QColor accent(e.color);
        p.setPen(QPen(selected ? QColor("#ffffff") : QColor("#596273"), selected ? 2 : 1));
        p.setBrush(QColor("#303743"));
        p.drawRoundedRect(r, 7, 7);

        if (e.type == "knob") {
            const int d = qMax(12, qMin(r.width() - 18, r.height() - 30));
            QRect dial(r.center().x() - d / 2, r.top() + 6, d, d);
            p.setPen(QPen(QColor("#9ba8ba"), 2));
            p.setBrush(QColor("#171b21"));
            p.drawEllipse(dial);
            const double range = qMax(0.000001, e.maximum - e.minimum);
            const double ratio = qBound(0.0, (e.value - e.minimum) / range, 1.0);
            const double angle = (-135.0 + 270.0 * ratio) * M_PI / 180.0;
            QPoint c = dial.center();
            QPoint needle(c.x() + int(qCos(angle) * d * .34),
                          c.y() + int(qSin(angle) * d * .34));
            p.setPen(QPen(accent, 3, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(c, needle);
        } else if (e.type == "slider") {
            const int cy = r.center().y();
            p.setPen(QPen(QColor("#171b21"), 5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(r.left() + 10, cy, r.right() - 10, cy);
            const double range = qMax(0.000001, e.maximum - e.minimum);
            const double ratio = qBound(0.0, (e.value - e.minimum) / range, 1.0);
            const int kx = r.left() + 10 + int(ratio * qMax(1, r.width() - 20));
            p.setPen(QPen(accent, 3, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(r.left() + 10, cy, kx, cy);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#e6edf7"));
            p.drawEllipse(QPoint(kx, cy), 6, 6);
        } else {
            p.setPen(Qt::NoPen);
            p.setBrush(accent.darker(150));
            p.drawRoundedRect(r.adjusted(8, 8, -8, -22), 5, 5);
            p.setPen(QPen(accent));
            p.drawRoundedRect(r.adjusted(8, 8, -8, -22), 5, 5);
        }

        QFont f = p.font();
        f.setPointSize(e.fontSize);
        p.setFont(f);
        p.setPen(QColor(e.textColor));
        p.drawText(QRect(r.left() + 3, r.bottom() - 20, r.width() - 6, 17),
                   Qt::AlignCenter, e.label);

        if (selected) {
            p.setPen(QPen(QColor("#ffffff"), 1));
            p.setBrush(QColor("#61a8ff"));
            p.drawRect(r.right() - 7, r.bottom() - 7, 7, 7);
        }
    }
}

void InstrumentCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;
    const QPoint point = event->position().toPoint();
    const int idx = elementAt(point);
    if (idx < 0) {
        m_selectedId.clear();
        emit elementSelected({});
        update();
        return;
    }

    const auto &e = m_elements.at(idx);
    const QRect r = elementRect(e);
    m_selectedId = e.id;
    m_draggedId = e.id;
    m_resizeStart = point;
    // Store the element's logical size once at mouse-down. Using the changing
    // size during mouseMove causes cumulative acceleration while resizing.
    m_resizeOriginal = QSize(e.width, e.height);
    if (onResizeHandle(r, point)) {
        m_resizing = true;
        m_dragging = false;
    } else {
        m_dragging = true;
        m_resizing = false;
        m_dragOffset = point - r.topLeft();
    }
    emit elementSelected(m_selectedId);
    update();
}

void InstrumentCanvas::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint point = event->position().toPoint();
    for (const auto &e : m_elements) {
        if (e.id != m_draggedId) continue;
        const QPoint logical = canvasPoint(point);
        if (m_resizing && (event->buttons() & Qt::LeftButton)) {
            const QPoint startLogical = canvasPoint(m_resizeStart);
            // Always calculate from the original size, not the previous event.
            const int w = qBound(20, m_resizeOriginal.width()
                                     + logical.x() - startLogical.x(), 1000);
            const int h = qBound(20, m_resizeOriginal.height()
                                     + logical.y() - startLogical.y(), 1000);
            emit elementResized(e.id, e.x, e.y, w, h);
        } else if (m_dragging && (event->buttons() & Qt::LeftButton)) {
            const QPoint topLeft = point - m_dragOffset;
            const QPoint logicalTopLeft = canvasPoint(topLeft);
            emit elementMoved(e.id, qMax(0, logicalTopLeft.x()), qMax(0, logicalTopLeft.y()));
        }
        break;
    }
}

void InstrumentCanvas::mouseReleaseEvent(QMouseEvent *)
{
    m_dragging = false;
    m_resizing = false;
    m_draggedId.clear();
}

void InstrumentCanvas::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-syntheditor-element"))
        event->acceptProposedAction();
}

void InstrumentCanvas::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-syntheditor-element"))
        event->acceptProposedAction();
}

void InstrumentCanvas::dropEvent(QDropEvent *event)
{
    const QString type = QString::fromUtf8(
        event->mimeData()->data("application/x-syntheditor-element"));
    if (type != "knob" && type != "slider" && type != "button") return;
    const QPoint logical = canvasPoint(event->position().toPoint());
    emit elementDropped(type, qMax(0, logical.x()), qMax(0, logical.y()));
    event->acceptProposedAction();
}

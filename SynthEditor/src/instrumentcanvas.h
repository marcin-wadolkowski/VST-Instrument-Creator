#pragma once

#include "instrumentmodel.h"

#include <QWidget>
#include <QImage>

class InstrumentCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit InstrumentCanvas(QWidget *parent = nullptr);

    void setElements(const QVector<InstrumentElement> &elements);
    void setSelectedId(const QString &id);
    QString selectedId() const;
    void setCanvasBackground(const QString &color, const QString &imagePath);
    void setCanvasLogicalSize(int width, int height);
    int logicalWidth() const { return m_logicalWidth; }
    int logicalHeight() const { return m_logicalHeight; }

signals:
    void elementSelected(const QString &id);
    void elementMoved(const QString &id, int x, int y);
    void elementResized(const QString &id, int x, int y, int width, int height);
    void elementDropped(const QString &type, int x, int y);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QRect elementRect(const InstrumentElement &element) const;
    int elementAt(const QPoint &point) const;
    bool onResizeHandle(const QRect &rect, const QPoint &point) const;
    QPoint canvasPoint(const QPoint &widgetPoint) const;

    QVector<InstrumentElement> m_elements;
    QString m_selectedId;
    QString m_draggedId;
    QPoint m_dragOffset;
    QPoint m_resizeStart;
    QSize m_resizeOriginal;
    bool m_dragging = false;
    bool m_resizing = false;
    int m_logicalWidth = 800;
    int m_logicalHeight = 450;
    QString m_backgroundColor = "#20242b";
    QString m_backgroundImagePath;
    QImage m_backgroundImage;
};

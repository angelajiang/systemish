#ifndef RASTERWINDOW_H
#define RASTERWINDOW_H

#include <QtGui>

class RasterWindow : public QWindow
{
    Q_OBJECT
public:
    explicit RasterWindow(QWindow *parent = 0);

    virtual void render(QPainter *painter);

public slots:
    void renderLater();
    void renderNow();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void exposeEvent(QExposeEvent *event) Q_DECL_OVERRIDE;

private:
    QBackingStore *m_backingStore;
    bool m_update_pending;
};
#endif // RASTERWINDOW_H

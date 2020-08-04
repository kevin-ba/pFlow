#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = nullptr);
    bool loadFile(const QString &);
    void saveFile();

private slots:
    void open();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();
    void mousePressEvent(QMouseEvent *event);

private:
    void createActions();
    void createMenus();
    void updateActions();
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void drawPolygon();

    QImage image;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor = 1;

    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;

    QPolygonF polygonPoints;
    QPolygonF polygonDoor;
    QList<QPolygonF> polygonDoorsList;
};

#endif // IMAGEVIEWER_H

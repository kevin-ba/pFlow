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
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void createActions();
    void createMenus();
    void updateActions();
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void drawPolygon();
    void reset();
    void remove();
    QPoint getClosestPoint(QPoint newPosition, QList<QPolygon> polyList);
    void insert();
    void insertNewPoint(QPoint newPoint);
    float distToSegment(QPoint newPoint, QPoint p1, QPoint p2);

    QImage image;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    int osOffset = 0;

    double scaleFactor = 1;

    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    QAction *removeAct;

    QPolygon polygonPoints;
    QPolygon polygonDoor;
    QList<QPolygon> polygonDoorsList;

    int iPoint;
    int iList;
    QPoint closestPoint;
    bool removePoint = false;
    bool leftClick = false;
    bool rightClick = false;

    bool insertPoint = false;
};

#endif // IMAGEVIEWER_H

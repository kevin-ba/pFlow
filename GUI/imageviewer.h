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
    void exportFile();
    bool importFile(const QString &);

private slots:
    void open();
    void openTxt();
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
    void newPoly(QString color);
    void remove();
    QPoint getClosestPoint(QPoint newPosition, QList<QPolygon> );
    void insert();
    void insertNewPoint(QPoint newPoint);
    float distToSegment(QPoint newPoint, QPoint p1, QPoint p2);

    QImage image;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    int osOffset = 20;

    double scaleFactor = 1;

    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    QAction *removeAct;
    QAction *importFileAct;
    QAction *exportFileAct;
    QAction *insertAct;
    QAction *resetAct;

    QPolygon polygonDoor;
    QList<QPolygon> polygonDoorsList;

    int iPoint;
    int iList;
    QPoint closestPoint;
    bool removePoint = false;
    bool leftClick = false;
    bool rightClick = false;

    bool insertPoint = false;

    // New Polygon
    void newPolyGreen();
    void newPolyRed();
    void newPolyBlue();

    QAction *newPolyGreenAct;
    QAction *newPolyRedAct;
    QAction *newPolyBlueAct;

    QList<QString> polyCount = {"blue"};
    QList<QPolygon> polyList;


    int lineWidth = 1;
    int pointWidth = 2;
    void increaseLine();
    void increasePoint();
    void decreaseLine();
    void decreasePoint();

    QAction *incLineAct;
    QAction *incPointAct;
    QAction *decLineAct;
    QAction *decPointAct;


};

#endif // IMAGEVIEWER_H

#include "imageviewer.h"
#include <QGuiApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include <QMessageBox>
#include <QAction>
#include <QMenuBar>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <qpainter.h>
#include <QtMath>
#include <limits>

ImageViewer::ImageViewer(QWidget *parent)
   : QMainWindow(parent), imageLabel(new QLabel)
   , scrollArea(new QScrollArea)
{
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    if (QSysInfo::productType() == "windows" || QSysInfo::productType() == "android" ){
        osOffset = 20;
    }

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void ImageViewer::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

bool ImageViewer::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }
    image = newImage;
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    fitToWindowAct->setEnabled(true);
    updateActions();
    QPixmap p = QPixmap::fromImage(image);

    imageLabel->setPixmap(p);
    imageLabel->show();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();

    return true;
}

void ImageViewer::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save File"), "",
            tr("ASCII-File (*.txt)"));
    if (fileName.isEmpty())
            return;
    else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(this, tr("Unable to open file"),
                file.errorString());
            return;
        }

        QTextStream out(&file);
                int x = 0;
                foreach(QPoint point, polygonPoints)
                {
                    out << QStringLiteral("P%1\t%2\t%3\n").arg(x).arg(QString::number(point.x())).arg(QString::number(point.y()));
                    x++;
                }
                int i = 0;
                foreach(QPolygon door, polygonDoorsList)
                {
                    foreach(QPoint point, door)
                    {
                        out << QStringLiteral("D%1\t%2\t%3\n").arg(i).arg(QString::number(point.x())).arg(QString::number(point.y()));
                    }
                    i++;
                }
     }
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        normalSize();
    updateActions();
}

void ImageViewer::about()
{
    QMessageBox::about(this, tr("About *Appname*"),
            tr("<p><b>Normal-Point:</b> Leftclick "
               "<p><b>Door-Point:</b> Rightclick "
               "<p><b>Change normal point:</b> Click and hold left mouse button on point"
               " -> Move point to new location -> Release left mouse button</p>"
               "<p><b>Change door point:</b> Click and hold right mouse button on point"
               " -> Move point to new location -> Release right mouse button</p>"
               "<p><b>Insert:</b> Edit -> Insert -> Click between segement</p>"
               "</p><b>Remove:</b> Double click on point -> Edit -> Remove </p>"
               "<p>Hier können weitere Infos stehen</p>"));
}

void ImageViewer::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    QAction *saveAct = fileMenu->addAction(tr("&Save..."), this, &ImageViewer::saveFile);
    saveAct->setShortcut(tr("Ctrl+S"));

    QAction *resetAct = fileMenu->addAction(tr("&Reset"), this, &ImageViewer::reset);
    resetAct->setShortcut(tr("Ctrl+R"));

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    removeAct = editMenu->addAction(tr("&Remove"), this, &ImageViewer::remove);
    removeAct->setShortcut(QKeySequence::Delete);
    removeAct->setEnabled(false);

    QAction *insertAct = editMenu->addAction(tr("&Insert Point"), this, &ImageViewer::insert);
    insertAct->setShortcut(tr("Ctrl+I"));


    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+V"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void ImageViewer::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void ImageViewer::scaleImage(double factor)
{
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap(Qt::ReturnByValue).size());



    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPoint mousePoint = imageLabel->mapFromParent(event->pos());

    QPoint mousePointReal;
    mousePointReal.setX((mousePoint.x()) / scaleFactor);
    mousePointReal.setY((mousePoint.y() - osOffset) / scaleFactor);

    if(!removePoint)
    {
        if(event->buttons() & Qt::LeftButton)
        {
            leftClick = true;
            rightClick = false;
            QList<QPolygon> polyList;
            if(polygonPoints.length() >= 1)
                polyList.append(polygonPoints);
            closestPoint = getClosestPoint(mousePointReal, polyList);
        }
        else if(event->buttons() & Qt::RightButton)
        {
            rightClick = true;
            leftClick = false;
            closestPoint = getClosestPoint(mousePointReal, polygonDoorsList);
        }

        if((mousePointReal - closestPoint).manhattanLength() < 20)
        {
            removePoint = true;
            removeAct->setEnabled(true);
        }
    }
    else
    {
        removePoint = false;
        removeAct->setEnabled(false);
    }

    drawPolygon();
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if(!insertPoint)
    {
        QPoint mousePoint = imageLabel->mapFromParent(event->pos());

        QPoint mousePointReal;
        mousePointReal.setX((mousePoint.x()) / scaleFactor);
        mousePointReal.setY((mousePoint.y() - osOffset) / scaleFactor);

        if(event->buttons() & Qt::LeftButton)
        {
            QList<QPolygon> polyList;
            if(polygonPoints.length() >= 1)
                polyList.append(polygonPoints);

            closestPoint = getClosestPoint(mousePointReal, polyList);
            if((mousePointReal - closestPoint).manhattanLength() < 50)
                polygonPoints.replace(iPoint, mousePointReal);
        }
        else if(event->buttons() & Qt::RightButton)
        {
            closestPoint = getClosestPoint(mousePointReal, polygonDoorsList);
            if((mousePointReal - closestPoint).manhattanLength() < 50)
            {
                QPolygon z = polygonDoorsList.takeAt(iList);
                z.replace(iPoint, mousePointReal);
                polygonDoorsList.insert(iList, z);
            }
        }

        drawPolygon();
    }
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    QPoint mousePoint = imageLabel->mapFromParent(event->pos());

    QPoint mousePointReal;
    mousePointReal.setX((mousePoint.x()) / scaleFactor);
    mousePointReal.setY((mousePoint.y() - osOffset) / scaleFactor);

    if(event->button() == Qt::LeftButton)
    {
        if(!insertPoint)
        {
            QList<QPolygon> polyList;
            if(polygonPoints.length() >= 1)
                polyList.append(polygonPoints);

            closestPoint = getClosestPoint(mousePointReal, polyList);
            if((mousePointReal - closestPoint).manhattanLength() > 10)
            {
                polygonPoints << mousePointReal;
            }
        }
        else
        {
            insertNewPoint(mousePointReal);
        }
    }
    if(event->button() == Qt::RightButton)
    {
        closestPoint = getClosestPoint(mousePointReal, polygonDoorsList);
        if((mousePointReal - closestPoint).manhattanLength() > 10)
        {
            if(polygonDoorsList.length() >= 1)
            {
                if(polygonDoorsList.last().length() == 1)
                {
                    polygonDoorsList.last() << mousePointReal;
                }
                else
                {
                    QPolygon poly;
                    poly << mousePointReal;
                    polygonDoorsList << poly;
                }
            }
            else
            {
                QPolygon poly;
                poly << mousePointReal;
                polygonDoorsList << poly;
            }
        }
    }

    drawPolygon();
}

void ImageViewer::drawPolygon()
{
    QImage tmp(image);
    QPainter *painter = new QPainter(&tmp);

    QPen pen(Qt::blue, 1);
    painter->setPen(pen);
    painter->drawPolygon(polygonPoints);

    pen = QPen(Qt::black, 2);
    painter->setPen(pen);
    painter->drawPoints(polygonPoints);

    //alle Türen einzeichnen
    foreach(QPolygon door, polygonDoorsList)
    {
        pen = QPen(Qt::magenta, 1);
        painter->setPen(pen);
        painter->drawPolygon(door);

        pen = QPen(Qt::black, 2);
        painter->setPen(pen);
        painter->drawPoints(door);
    }

    if(removePoint)
    {
        pen = QPen(Qt::red, 3);
        painter->setPen(pen);
        painter->drawPoint(closestPoint);
    }

    painter->end();
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
}

void ImageViewer::reset()
{
    polygonPoints.clear();
    polygonDoorsList.clear();
    imageLabel->setPixmap(QPixmap::fromImage(image));
    removePoint = false;
    removeAct->setEnabled(false);
    insertPoint = false;
}

QPoint ImageViewer::getClosestPoint(QPoint newPosition, QList<QPolygon> polyList)
{
    QPoint closestPoint;
    float manhattenLength = std::numeric_limits<float>::max();

    iPoint = 0;
    iList = 0;
    int iL = 0;
    foreach(QPolygon poly, polyList)
    {
        int iP = 0;
        foreach(QPoint point, poly)
        {
            float mhL = (point - newPosition).manhattanLength();
            if(mhL < manhattenLength)
            {
                closestPoint = point;
                manhattenLength = mhL;
                iPoint = iP;
                iList = iL;
            }
            iP++;
        }
        iL++;
    }

    return closestPoint;
}

void ImageViewer::remove(){
    if(leftClick)
    {
        polygonPoints.removeAt(iPoint);
        leftClick = false;
    }
    else if(rightClick)
    {
        polygonDoorsList.removeAt(iList);
        rightClick = false;
    }

    removePoint = false;
    removeAct->setEnabled(false);
    drawPolygon();
}

void ImageViewer::insert()
{
    if(insertPoint == false)
        insertPoint = true;
    else
        insertPoint = false;
}

void ImageViewer::insertNewPoint(QPoint newPoint)
{
    int index = 0;
    int j;
    float minDist = std::numeric_limits<float>::max();
    float dist;

    for(int i = 0; i < polygonPoints.length(); i++)
    {
        if(i < polygonPoints.length() - 1)
            j = i + 1;
        else
            j = 0;

        dist = distToSegment(newPoint, polygonPoints.at(i), polygonPoints.at(j));
        if(dist < minDist)
        {
            minDist = dist;
            index = i;
        }
    }

    if(index == 0)
        polygonPoints << newPoint;
    else
        polygonPoints.insert(index+1, newPoint);

    insertPoint = false;
}

float ImageViewer::distToSegment(QPoint newPoint, QPoint p1, QPoint p2)
{
    int x1 = p1.x();
    int y1 = p1.y();
    int x2 = p2.x();
    int y2 = p2.y();
    int x3 = newPoint.x();
    int y3 = newPoint.y();

    float px=x2-x1;
    float py=y2-y1;
    float temp=(px*px)+(py*py);
    float u=((x3 - x1) * px + (y3 - y1) * py) / (temp);

    if(u>1)
    {
        u=1;
    }
    else if(u<0)
    {
        u=0;
    }

    float x = x1 + u * px;
    float y = y1 + u * py;

    float dx = x - x3;
    float dy = y - y3;

    return qSqrt(dx*dx + dy*dy);

}

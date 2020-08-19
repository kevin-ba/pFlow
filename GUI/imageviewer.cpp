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

    if (QSysInfo::productType() == "osx"){
        osOffset = 0;
    }

    polyList.append(QPolygon());

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode, QString typeFilter, QString defaultSuffix)
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
    dialog.selectMimeTypeFilter(typeFilter);
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix(defaultSuffix);
}

static void initializeTextFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode, QString typeFilter, QString defaultSuffix)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList fileLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
        dialog.setDirectory(fileLocations.isEmpty() ? QDir::currentPath() : fileLocations.last());
    }

    dialog.selectMimeTypeFilter(typeFilter);
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix(defaultSuffix);
}

void ImageViewer::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen, "image/jpeg", "jpg");

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
    QPixmap p = QPixmap::fromImage(image);

    imageLabel->setPixmap(p);
    imageLabel->show();

    imageLabel->adjustSize();

    ImageViewer::reset();

    return true;
}

void ImageViewer::openTxt()
{
    QFileDialog dialog(this, tr("Import File"), QDir::currentPath(), tr("ASCII-File (*.dat)"));
    initializeTextFileDialog(dialog, QFileDialog::AcceptOpen, "text/plain", "dat");

    while (dialog.exec() == QDialog::Accepted && !importFile(dialog.selectedFiles().first())) {}
}

bool ImageViewer::importFile(const QString &fileName)
{

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    ImageViewer::reset();
    polyList.clear();
    polyCount.clear();
    polygonDoorsList.clear();

    QTextStream in(&file);

    QStringList stringList;
    bool isPoly;
    int countP = 0;
    int countD = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
       if(line.startsWith("P")){
           isPoly = true;
           stringList = line.split(", ");
           polyCount.append(stringList[1]);
           countP++;
           polyList.append(QPolygon());
       } else if(line.startsWith("D")){
           isPoly = false;
           countD++;
           polygonDoorsList.append(QPolygon());
       } else {
           if(isPoly){
               stringList = line.split(" ");
               polyList[countP-1].append(QPoint(stringList[0].toFloat(), stringList[1].toFloat()));
           }else if(!isPoly){
               stringList = line.split(" ");
               polygonDoorsList[countD-1].append(QPoint(stringList[0].toFloat(), stringList[1].toFloat()));
           }
       }
    }

    ImageViewer::drawPolygon();
    return true;
}

void ImageViewer::exportFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Export File"), "",
            tr("ASCII-File (*.dat)"));
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
                int y = 0;
                foreach(QPolygon poly, polyList)
                {
                    out << QStringLiteral("P, %1, %2\n").arg(polyCount[x]).arg(polyList[x].length());
                    foreach(QPoint point, poly)
                    {
                        out << QStringLiteral("%1 %2\n").arg(QString::number(point.x())).arg(QString::number(point.y()));
                        y++;
                    }
                    x++;
                }
                int i = 0;
                foreach(QPolygon door, polygonDoorsList)
                {
                    out << QStringLiteral("D\n");
                    foreach(QPoint point, door)
                    {
                        out << QStringLiteral("%1 %2\n").arg(QString::number(point.x())).arg(QString::number(point.y()));
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

void ImageViewer::about()
{
    QMessageBox::about(this, tr("About *Appname*"),
            tr("<p><b>Normal-Point:</b> Left click "
               "<p><b>Door-Point:</b> Right click "
               "<p><b>Change normal point:</b> Click and hold left mouse button on point"
               " -> Move point to new location -> Release left mouse button</p>"
               "<p><b>Change door point:</b> Click and hold right mouse button on point"
               " -> Move point to new location -> Release right mouse button</p>"
               "<p><b>Insert:</b> Edit -> Insert -> Click near segement where you want to insert</p>"
               "</p><b>Remove:</b> Double click on point (point turns red) -> Edit -> Remove </p>"
               "</p><b>Cancel Remove:</b> Double click on point (point turns black)</p>"
               "</p><b>New polygon:</b> Edit -> New Polygon (Color)</p>"
               "</p><b>Reset all points:</b> File -> Reset</p>"));
}

void ImageViewer::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    QAction *importAct = fileMenu->addAction(tr("&Import..."), this, &ImageViewer::openTxt);
    importAct->setShortcut(tr("Ctrl+N"));

    QAction *exportAct = fileMenu->addAction(tr("&Export..."), this, &ImageViewer::exportFile);
    exportAct->setShortcut(tr("Ctrl+S"));

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

    editMenu->addSeparator();

    QAction *newPolyGreenAct = editMenu->addAction(tr("&New Polygon (Green)"), this, &ImageViewer::newPolyGreen);
    newPolyGreenAct->setShortcut(tr("Ctrl+1"));

    QAction *newPolyRedAct = editMenu->addAction(tr("&New Polygon (Red)"), this, &ImageViewer::newPolyRed);
    newPolyRedAct->setShortcut(tr("Ctrl+2"));

    QAction *newPolyBlueAct = editMenu->addAction(tr("&New Polygon (Blue)"), this, &ImageViewer::newPolyBlue);
    newPolyBlueAct->setShortcut(tr("Ctrl+3"));

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+V"));

    viewMenu->addSeparator();

    incLineAct = viewMenu->addAction(tr("&Increase Line Width"), this, &ImageViewer::increaseLine);
    incLineAct->setShortcut(tr("Ctrl+6"));

    decLineAct = viewMenu->addAction(tr("&Decrease Line Width"), this, &ImageViewer::decreaseLine);
    decLineAct->setShortcut(tr("Ctrl+7"));

    incPointAct = viewMenu->addAction(tr("&Increase Point Width"), this, &ImageViewer::increasePoint);
    incPointAct->setShortcut(tr("Ctrl+8"));

    decPointAct = viewMenu->addAction(tr("&Decrease Point Width"), this, &ImageViewer::decreasePoint);
    decPointAct->setShortcut(tr("Ctrl+9"));


    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
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

void ImageViewer::newPolyGreen()
{
    ImageViewer::newPoly("#00ff00");
}

void ImageViewer::newPolyRed()
{
    ImageViewer::newPoly("#ff0000");
}

void ImageViewer::newPolyBlue()
{
    ImageViewer::newPoly("#0000ff");
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
            closestPoint = getClosestPoint(mousePointReal, polyList);
        }
        else if(event->buttons() & Qt::RightButton)
        {
            rightClick = true;
            leftClick = false;
            closestPoint = getClosestPoint(mousePointReal, polygonDoorsList);
        }

        if((mousePointReal - closestPoint).manhattanLength() < 7)
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
            closestPoint = getClosestPoint(mousePointReal, polyList);
            if((mousePointReal - closestPoint).manhattanLength() < 50)
                polyList[iList].replace(iPoint, mousePointReal);
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
            closestPoint = getClosestPoint(mousePointReal, polyList);
            if((mousePointReal - closestPoint).manhattanLength() > 7)
            {
                polyList[polyCount.length()-1] << mousePointReal;
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
        if((mousePointReal - closestPoint).manhattanLength() > 7)
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
    QPen pen;
    int c = 0;
    foreach(QPolygon poly, polyList)
    {
        QColor color;
        color.setNamedColor(polyCount[c]);
        pen = QPen(color, lineWidth);
        painter->setPen(pen);
        painter->drawPolygon(poly);

        pen = QPen(Qt::black, pointWidth);
        painter->setPen(pen);
        painter->drawPoints(poly);
        c++;
    }

    foreach(QPolygon door, polygonDoorsList)
    {
        pen = QPen(Qt::magenta, lineWidth);
        painter->setPen(pen);
        painter->drawPolygon(door);

        pen = QPen(Qt::black, pointWidth);
        painter->setPen(pen);
        painter->drawPoints(door);
    }

    if(removePoint)
    {
        pen = QPen(Qt::red, pointWidth);
        painter->setPen(pen);
        painter->drawPoint(closestPoint);
    }

    painter->end();
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
}

void ImageViewer::reset()
{
    polyList.clear();
    polyCount.clear();
    polygonDoorsList.clear();
    imageLabel->setPixmap(QPixmap::fromImage(image));
    removePoint = false;
    removeAct->setEnabled(false);
    insertPoint = false;
    polyList.append(QPolygon());
    polyCount.append("#0000ff");
}

QPoint ImageViewer::getClosestPoint(QPoint newPosition, QList<QPolygon> list)
{
    QPoint closestPoint;
    float manhattenLength = std::numeric_limits<float>::max();

    iPoint = 0;
    iList = 0;
    int iL = 0;
    foreach(QPolygon poly, list)
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
        polyList[iList].removeAt(iPoint);
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
    int c = 0;

    foreach(QPolygon poly, polyList){
        for(int i = 0; i < poly.length(); i++)
        {
            if(i < poly.length() - 1)
                j = i + 1;
            else
                j = 0;

            dist = distToSegment(newPoint, poly.at(i), poly.at(j));
            if(dist < minDist)
            {
                minDist = dist;
                index = i;
                iList = c;
            }
        }
        c++;
    }

    if(polyList[iList].empty()){
        polyList[iList] << newPoint;
    }else{
        polyList[iList].insert(index+1, newPoint);
    }

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

void ImageViewer::newPoly(QString color)
{
    polyCount.append(color);
    polyList.append(QPolygon());
}

void ImageViewer::increaseLine(){
    if(lineWidth < 7)
        lineWidth++;
    drawPolygon();
}

void ImageViewer::increasePoint(){
    if(pointWidth < 10)
        pointWidth++;
    drawPolygon();
}

void ImageViewer::decreaseLine(){
    if(lineWidth > 1)
        lineWidth--;
    drawPolygon();
}

void ImageViewer::decreasePoint(){
    if(pointWidth > 1)
        pointWidth--;
    drawPolygon();
}

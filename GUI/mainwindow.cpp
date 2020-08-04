#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loadPictureButton_clicked()
{
    const QString picturesLocations = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    loadFile(picturesLocations);
}

void MainWindow::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
    }

    QGraphicsScene* scene = new QGraphicsScene();
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(newImage));
    scene->addItem(item);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
    ui->graphicsView->show();
}

QPolygonF polygonPoints;
QPolygonF polygonDoor;
QList<QPolygonF> polygonDoorsList;
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        {
            QPointF mousePoint = ui->graphicsView->mapToScene(event->pos());
            if(ui->radioButton_Points->isChecked())
            {
                polygonPoints << mousePoint;
                qDebug() << polygonPoints;
            }
            if(ui->radioButton_Door->isChecked())
            {
                polygonDoor << mousePoint;
                if(polygonDoor.length() == 2)
                {
                    polygonDoorsList.append(polygonDoor);
                    polygonDoor.clear();
                    qDebug() << polygonDoorsList;
                }
            }

           // drawPolygon();
        }
}

void MainWindow::drawPolygon()
{
    // pixmap von graphicsview scene holen und damit painter initalisieren
    QPainter *painter = new QPainter(this); // new QPainter(&pixmap);
    QPen pen(Qt::blue, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->drawPolygon(polygonPoints);
    /*alle Türen einzeichnen
    pen = QPen(Qt::red, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    foreach(QPolygonF door, polygonDoorsList)
    {
        painter->drawPolygon(door);
    }*/
    // pixmap item von graphicsview scene updaten
}

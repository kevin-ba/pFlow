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

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        {
            QPointF mousePoint = ui->graphicsView->mapToScene(event->pos());
            qDebug() << mousePoint;
            // punkt abspeichern um polygonzug zu erstellen
            // punkt bzw. polygonzug zeichnen
        }
}

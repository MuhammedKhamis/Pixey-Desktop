#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QScrollArea>
#include <bits/stdc++.h>
#include <QFileDialog>
#include <QMouseEvent>
#include <QRubberBand>
#include <QScrollBar>
#include <imageobject.h>
#include <QLayout>
#include <QPainter>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    current_rotation_angle = 0;
    current_size_factor = 1;

    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    image_label = ui->label;
    image_label->setBackgroundRole(QPalette::Light);

    image_label->setAlignment(Qt::AlignTop | Qt::AlignLeft);


    scroll_area = ui->scrollArea;

    scroll_area->setBackgroundRole(QPalette::Dark);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);


    setWindowTitle("Image Editor");



    ui->slider->setOrientation(Qt::Vertical);
    ui->slider->setRange(30, 300);
    ui->slider->setValue(100);
    ui->slider->setEnabled(false);

    ui->rotation_dial->setRange(0, 360);
    ui->rotation_dial->setValue(0);
    ui->rotation_dial->setEnabled(false);

    this->slider = ui->slider;
    this->rotation_dial = ui->rotation_dial;


    connect(rotation_dial, SIGNAL(valueChanged(int)), this, SLOT(rotate(int)));
    connect(rotation_dial, SIGNAL(sliderReleased()), this, SLOT(save_rotation_image()));

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(zoom(int)));
    connect(slider, SIGNAL(sliderReleased()), this, SLOT(save_zoomed_image()));

    import_button = new QAction(tr("&Import"));
    import_button->setShortcut(tr("Ctrl+O"));

    export_button = new QAction(tr("&Save"));
    export_button->setShortcut(tr("Ctrl+S"));

    exit_button = new QAction(tr("&Exit"));
    exit_button->setShortcut(tr("Ctrl+E"));


    QMenu *file_menu = new QMenu(tr("&File"));
    file_menu->addAction(import_button);
    file_menu->addAction(export_button);
    file_menu->addSeparator();
    file_menu->addAction(exit_button);


    zoom_in_button = new QAction(tr("&Zoom In"));
    zoom_in_button->setShortcut(tr("Ctrl++"));
    zoom_in_button->setEnabled(false);
    connect(zoom_in_button, SIGNAL(triggered()), this, SLOT(zoom_in_percentage()));

    zoom_out_button = new QAction(tr("&Zoom Out"));
    zoom_out_button->setShortcut(tr("Ctrl+-"));
    zoom_out_button->setEnabled(false);
    connect(zoom_out_button, SIGNAL(triggered()), this, SLOT(zoom_out_percentage()));



    connect(import_button, SIGNAL(triggered()), this, SLOT(import()));
    connect(export_button, SIGNAL(triggered()), this, SLOT(save()));
    connect(exit_button, SIGNAL(triggered()), this, SLOT(close()));



    crop_button = new QAction(tr("&Crop"));
    crop_button->setShortcut(tr("Ctrl+C"));
    crop_button->setEnabled(false);
    connect(crop_button, SIGNAL(triggered()), this, SLOT(crop()));

    undo_button = new QAction(tr("&Undo"));
    undo_button->setShortcut(tr("Ctrl+Z"));
    connect(undo_button, SIGNAL(triggered()), this, SLOT(undo()));


    redo_button = new QAction(tr("&Redo"));
    redo_button->setShortcut(tr("Ctrl+Y"));
    connect(redo_button, SIGNAL(triggered()), this, SLOT(redo()));

    QMenu *view_menu = new QMenu(tr("&View"));
    view_menu->addAction(crop_button);
    view_menu->addAction(undo_button);
    view_menu->addAction(redo_button);
    view_menu->addSeparator();
    view_menu->addAction(zoom_in_button);
    view_menu->addAction(zoom_out_button);


    menuBar()->addMenu(file_menu);
    menuBar()->addMenu(view_menu);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::import()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath());
    if(fileName.isNull() || fileName.isEmpty())return;
    QImage image (fileName);
    if(image.isNull())return;
    insert_image(image);

    this->current_rotation_angle = 0;
    this->current_size_factor = 1;
    this->undoActions.push_back(new ImageObject(image, 1.0, 0));

    this->redoActions.clear();
}

void MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::homePath());
    if(fileName.isNull() || fileName.isEmpty() || image_label->pixmap() == NULL)return;
    QImage image (image_label->pixmap()->toImage());
    if(image.isNull())return;
    image.save(fileName);
}

void MainWindow::insert_image(QImage image){
    current_image = image;
    image_label->resize(image.width(), image.height());
    image_label->setPixmap(QPixmap::fromImage(current_image));
    original_image = new QPixmap(*(this->image_label->pixmap()));

    this->current_rotation_angle = 0;
    this->current_size_factor = 1;

    ui->rotation_dial->setValue(0);
    ui->slider->setValue(100);

    ui->slider->setEnabled(true);
    ui->rotation_dial->setEnabled(true);

    zoom_in_button->setEnabled(current_size_factor < 3);
    zoom_out_button->setEnabled(current_size_factor > 0.3);


}

void MainWindow::crop(){

    image_label->show();

    int x_value = rubberBand->geometry().x() - image_label->geometry().x() - scroll_area->geometry().x() + scroll_area->horizontalScrollBar()->value();
    int y_value = rubberBand->geometry().y() - image_label->geometry().y() - scroll_area->geometry().y() + scroll_area->verticalScrollBar()->value();
    int width_value = rubberBand->geometry().width();
    int height_value = rubberBand->geometry().height();

    rubberBand->setGeometry(x_value, y_value, width_value, height_value);


    if(current_image.isNull())return ;


    QRect *rectt = new QRect(rubberBand->geometry().topLeft(), rubberBand->geometry().size());
    QRect rect;
    rect = *rectt;
    if(rect.isNull()) return;


    QImage cropped_image = current_image.copy(rect);
    if(cropped_image.isNull()) return;

    rubberBand->hide();


    this->undoActions.push_back(new ImageObject(cropped_image, current_size_factor,current_rotation_angle));
    insert_image(cropped_image);
    makeViewsUnavailable();
    this->redoActions.clear();
}


int cnt = 0;

QPoint origin;
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    makeViewsUnavailable();
    if(!image_label->underMouse())return;

    origin = event->pos();
    if (!rubberBand)
        rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    rubberBand->setGeometry(QRect(origin, QSize()));
    rubberBand->show();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{


    QPoint *end_point = new QPoint(qMin(event->pos().x(), image_label->geometry().topLeft().x() + image_label->geometry().width()) ,
                                   qMin(event->pos().y(), image_label->geometry().topLeft().y() + image_label->geometry().height()) );
    rubberBand->setGeometry(QRect(origin, *end_point).normalized());
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (!current_image.isNull()
            && rubberBand->geometry().width() > 1 && rubberBand->geometry().height() > 1 ) {
        makeViewsAvailable();
    }
}


void MainWindow::makeViewsAvailable(){
    crop_button->setEnabled(true);
    undo_button->setEnabled(true);
}

void MainWindow::makeViewsUnavailable(){
    crop_button->setEnabled(false);
}

void MainWindow::close(){
    exit(0);
}


void MainWindow::undo(){
    if(this->undoActions.size() == 0)return;
    ImageObject * action = this->undoActions.back();
    this->undoActions.pop_back();

    this->redoActions.push_back(action);

    if(this->undoActions.size() == 0){
        this->image_label->clear();
        this->current_rotation_angle = 0;
        this->current_size_factor = 1.0;
        ui->slider->setEnabled(false);
        ui->rotation_dial->setEnabled(false);
        zoom_in_button->setEnabled(false);
        zoom_out_button->setEnabled(false);
        crop_button->setEnabled(false);
        return;
    }


    action = this->undoActions.back();

    this->current_image = action->image;
    this->current_size_factor = action->size_factor;
    this->current_rotation_angle = action->rotation_angle;

    ui->slider->setValue(current_size_factor * 100);

    insert_image(current_image);

}


void MainWindow::redo(){

    if(this->redoActions.size() == 0)return;
    ImageObject * action = this->redoActions.back();
    this->redoActions.pop_back();
    this->undoActions.push_back(action);

    this->current_image = action->image;
    this->current_size_factor = action->size_factor;
    this->current_rotation_angle = action->rotation_angle;
    ui->slider->setValue(current_size_factor * 100);


    insert_image(current_image);
}


void MainWindow::rotate(int value){

    if(image_label->pixmap() == NULL)return;

    double offset = value - current_rotation_angle;
    current_rotation_angle = current_rotation_angle + offset;

    QTransform transform;
    QTransform trans = transform.rotate(current_rotation_angle);
    QLabel *temp_label = new QLabel;

    temp_label->setPixmap(original_image->scaled((original_image->size() * current_size_factor)));
    temp_label->setPixmap(temp_label->pixmap()->transformed(trans));
    image_label->setPixmap(*temp_label->pixmap());
    rubberBand->hide();
}


void MainWindow::save_rotation_image(){
    if(image_label->pixmap() == NULL)return;

    this->current_image = image_label->pixmap()->toImage();
    this->undoActions.push_back(new ImageObject(image_label->pixmap()->toImage(), current_size_factor, current_rotation_angle));
    this->redoActions.clear();
    rubberBand->hide();

}


void MainWindow::zoom(int value){
    if(image_label->pixmap() == NULL)return;

    if(rubberBand != NULL && rubberBand->geometry().width() != 0 && rubberBand->geometry().height() != 0){
        zoom_selectingArea(value);
    }

    double factor = value/100.0;
    current_size_factor = factor;

    QLabel *temp_label = new QLabel;
    QTransform transform;
    QTransform trans = transform.rotate(current_rotation_angle);
    temp_label->setPixmap(original_image->scaled(original_image->size() * current_size_factor));
    temp_label->setPixmap(temp_label->pixmap()->transformed(trans));
    image_label->setPixmap(*temp_label->pixmap());

    zoom_in_button->setEnabled(current_size_factor < 3);
    zoom_out_button->setEnabled(current_size_factor > 0.3);
    rubberBand->hide();

}

void MainWindow::save_zoomed_image(){
    if(image_label->pixmap() == NULL)return;

    this->undoActions.push_back(new ImageObject(image_label->pixmap()->toImage(), current_size_factor, current_rotation_angle));
    this->redoActions.clear();

    this->current_image = image_label->pixmap()->toImage();

    //rubberBand->hide();
}

void MainWindow::zoom_in_percentage()
{
    double factor = 0.1;
    current_size_factor += factor;
    ui->slider->setValue(current_size_factor * 100);

    zoom_in_button->setEnabled(current_size_factor < 3);
    zoom_out_button->setEnabled(current_size_factor > 0.3);

    save_zoomed_image();

    return;

    image_label->resize(current_size_factor * image_label->pixmap()->size());

    scroll_area->horizontalScrollBar()->setValue(int(factor * scroll_area->horizontalScrollBar()->value()
                                + ((factor - 1) * scroll_area->horizontalScrollBar()->pageStep()/2)));

    scroll_area->verticalScrollBar()->setValue(int(factor * scroll_area->verticalScrollBar()->value()
                                + ((factor - 1) * scroll_area->verticalScrollBar()->pageStep()/2)));

}


void MainWindow::zoom_out_percentage()
{
    double factor = 0.1;
    current_size_factor -= factor;
    ui->slider->setValue(current_size_factor * 100);
    zoom_in_button->setEnabled(current_size_factor < 3);
    zoom_out_button->setEnabled(current_size_factor > 0.3);

    save_zoomed_image();
    return;

    image_label->resize(current_size_factor * image_label->pixmap()->size());

    scroll_area->horizontalScrollBar()->setValue(int(factor * scroll_area->horizontalScrollBar()->value()
                                + ((factor - 1) * scroll_area->horizontalScrollBar()->pageStep()/2)));

    scroll_area->verticalScrollBar()->setValue(int(factor * scroll_area->verticalScrollBar()->value()
                                + ((factor - 1) * scroll_area->verticalScrollBar()->pageStep()/2)));

}


void MainWindow::zoom_selectingArea(int value){
    int ratio = (rubberBand->geometry().width() / (image_label->pixmap()->width()));
    double factor = value/100.0 * ratio;

    current_size_factor = factor;

    QLabel *temp_label = new QLabel;
    QTransform transform;
    QTransform trans = transform.rotate(current_rotation_angle);
    temp_label->setPixmap(original_image->scaled(original_image->size() * current_size_factor));
    temp_label->setPixmap(temp_label->pixmap()->transformed(trans));
    image_label->setPixmap(*temp_label->pixmap());

    scroll_area->horizontalScrollBar()->setValue(rubberBand->geometry().topLeft().x());
    scroll_area->verticalScrollBar()->setValue(rubberBand->geometry().topLeft().y());

    zoom_in_button->setEnabled(current_size_factor < 3);
    zoom_out_button->setEnabled(current_size_factor > 0.3);

    rubberBand->hide();
}

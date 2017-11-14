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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    current_rotation_angle = 0;
    current_size_factor = 1;

    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    //image_label = ui->image_label;
    //image_label->hide();

    image_label = new QLabel;
    image_label->setBackgroundRole(QPalette::Light);
    //image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    //image_label->setScaledContents(true);
    image_label->resize(0,0);
    image_label->setAlignment(Qt::AlignTop | Qt::AlignLeft);


    //scroll_area = new QScrollArea;
    scroll_area = ui->scrollArea;

    scroll_area->setBackgroundRole(QPalette::Dark);
    scroll_area->setWidget(image_label);
    //scroll_area->setWidgetResizable(true);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);


    //ui->main_layout->addWidget(scroll_area);
    //setCentralWidget(scroll_area);


    /*
    ui->scroll_area->setBackgroundRole(QPalette::Dark);
    ui->scroll_area->setWidget(image_label);
    */

    setWindowTitle("Image Editor");


//////////////////////////////////////////////////
    ui->slider->setOrientation(Qt::Vertical);
    ui->slider->setRange(30, 300);
    ui->slider->setValue(100);

    ui->rotation_dial->setRange(0, 360);
    ui->rotation_dial->setValue(0);

    this->slider = ui->slider;
    this->rotation_dial = ui->rotation_dial;
//////////////////////

    connect(rotation_dial, SIGNAL(valueChanged(int)), this, SLOT(rotate(int)));
    connect(rotation_dial, SIGNAL(sliderReleased()), this, SLOT(save_rotation_image()));

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(zoom(int)));
    connect(slider, SIGNAL(sliderReleased()), this, SLOT(save_zoomed_image()));
////////////////////////////
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


    connect(import_button, SIGNAL(triggered()), this, SLOT(import()));
    connect(export_button, SIGNAL(triggered()), this, SLOT(save()));
    connect(exit_button, SIGNAL(triggered()), this, SLOT(close()));



    crop_button = new QAction(tr("&Crop"));
    crop_button->setShortcut(tr("Ctrl+C"));
    crop_button->setEnabled(false);
    connect(crop_button, SIGNAL(triggered()), this, SLOT(crop()));

    undo_button = new QAction(tr("&Undo"));
    undo_button->setShortcut(tr("Ctrl+Z"));
    //undo_button->setEnabled(false);
    connect(undo_button, SIGNAL(triggered()), this, SLOT(undo()));


    redo_button = new QAction(tr("&Redo"));
    redo_button->setShortcut(tr("Ctrl+Y"));
    //redo_button->setEnabled(false);
    connect(redo_button, SIGNAL(triggered()), this, SLOT(redo()));

    QMenu *view_menu = new QMenu(tr("&View"));
    view_menu->addAction(crop_button);
    view_menu->addAction(undo_button);
    view_menu->addAction(redo_button);


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

    this->undoActions.push_back(new ImageObject(image, 1, 0));

    this->redoActions.clear();
}

void MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::homePath());
    if(fileName.isNull() || fileName.isEmpty())return;
    QImage image (image_label->pixmap()->toImage());
    if(image.isNull())return;
    image.save(fileName);
}

void MainWindow::insert_image(QImage image){
    current_image = image;
    image_label->resize(image.width(), image.height());
    image_label->setPixmap(QPixmap::fromImage(current_image));
    original_image = new QPixmap(*(this->image_label->pixmap()));
}

void MainWindow::crop(){

    rubberBand->setGeometry(rubberBand->geometry().x() - scroll_area->geometry().x() + scroll_area->horizontalScrollBar()->value(),
                            rubberBand->geometry().y() - scroll_area->geometry().y() + scroll_area->verticalScrollBar()->value(),
                            rubberBand->geometry().width(), rubberBand->geometry().height());




    std::cerr << "lets crop the image\n";

    if(current_image.isNull()) std::cerr << "error image is null\n";


    QRect *rectt = new QRect(rubberBand->geometry().topLeft(), rubberBand->geometry().size());
    QRect rect;
    rect = *rectt;
    if(rect.isNull()) std::cerr << "error rect image is null\n";


    QImage cropped_image = current_image.copy(rect);
    if(cropped_image.isNull()) std::cerr << "error cropped image is null\n";

    rubberBand->hide();


    this->undoActions.push_back(new ImageObject(cropped_image, current_size_factor,current_rotation_angle));



    insert_image(cropped_image);

    makeViewsUnavailable();

    rubberBand = NULL;

    //after any action we clear redo as we start from new action so previous actions must be cleared.
    this->redoActions.clear();
}


int cnt = 0;

QPoint origin;
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    makeViewsUnavailable();
    std::cerr << "error" << cnt++ << "\n";

    //selecting outside the image
    if(!image_label->underMouse())return;

    origin = event->pos();
    if (!rubberBand)
        rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    rubberBand->setGeometry(QRect(origin, QSize()));
    rubberBand->show();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    rubberBand->setGeometry(QRect(origin, event->pos()).normalized());
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{

    //rubberBand->hide();

    //now we have selected part of image
    std::cerr << "released event" << cnt++ << "\n";
    if (!current_image.isNull()
            && rubberBand->geometry().width() > 1 && rubberBand->geometry().height() > 1 ) {
        std::cerr << "selected is done\n";
        makeViewsAvailable();
    }
}


//enable buttons after selecting part of image
void MainWindow::makeViewsAvailable(){
    crop_button->setEnabled(true);
    undo_button->setEnabled(true);
}

void MainWindow::makeViewsUnavailable(){
    crop_button->setEnabled(false);
    //undo_button->setEnabled(false);
}

void MainWindow::close(){
    exit(0);
}


void MainWindow::undo(){
    std::cerr << "undo  ,, size of undo vector = " << this->undoActions.size() << "\n";

    if(this->undoActions.size() == 0)return;

    //back is the current showing action so pop it and execute the previous one
    ImageObject * action = this->undoActions.back();
    this->undoActions.pop_back();

    this->redoActions.push_back(action);

    if(this->undoActions.size() == 0){
        // clear the image and reset every thing
        this->image_label->clear();
        return;
    }


    action = this->undoActions.back();

    //setting the attributes
    this->current_image = action->image;
    this->current_size_factor = action->size_factor;
    this->current_rotation_angle = action->rotation_angle;

    insert_image(current_image);

}


void MainWindow::redo(){
    std::cerr << "redo  ,, size of redo vector = " << this->redoActions.size() << "\n";

    if(this->redoActions.size() == 0)return;

    //back is the action we want to perfrom
    ImageObject * action = this->redoActions.back();
    this->redoActions.pop_back();
    this->undoActions.push_back(action);


    //setting the attributes
    this->current_image = action->image;
    this->current_size_factor = action->size_factor;
    this->current_rotation_angle = action->rotation_angle;

    insert_image(current_image);
}


void MainWindow::rotate(int value){
    std::cerr << "value in rotation = " << value <<"\n";

    double offset = value - current_rotation_angle;
    current_rotation_angle = current_rotation_angle + offset;

    QTransform transform;
    QTransform trans = transform.rotate(current_rotation_angle);
    QLabel *temp_label = new QLabel;
    temp_label->setPixmap(this->original_image->transformed(trans));
    image_label->setPixmap(*temp_label->pixmap());

}

void MainWindow::save_rotation_image(){
    this->undoActions.push_back(new ImageObject(image_label->pixmap()->toImage(), current_size_factor, current_rotation_angle));
    this->redoActions.clear();
}


void MainWindow::zoom(int value){
    Q_ASSERT(image_label->pixmap());

    double factor = value/100.0;
    current_size_factor *= factor;
    if(current_size_factor < 0.333 || current_size_factor > 3)return;
    std::cerr << "value in zoom = " << value << "  ,  factor " << factor << "current factor = " << current_size_factor << "\n";
    //image_label->setPixmap(image_label->pixmap()->scaled(factor *image_label->pixmap()->size()));

    image_label->resize(current_size_factor * image_label->pixmap()->size());


    //QSize size = image_label->pixmap()->size() * current_size_factor;
    //image_label->resize(size);
}

void MainWindow::save_zoomed_image(){

}



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>
#include <QMenu>
#include <QRubberBand>
#include <imageobject.h>
#include <QGridLayout>
#include <QSlider>
#include <QDial>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QLabel *image_label;
    QScrollArea *scroll_area;
    QRubberBand *rubberBand;
    QImage current_image;
    QSlider *slider;
    QDial * rotation_dial;
    QPixmap *original_image;


    QAction *import_button;
    QAction *export_button;
    QAction *exit_button;

    QAction *crop_button;
    QAction *undo_button;
    QAction *redo_button;


    double current_size_factor;
    int current_rotation_angle;

    // pair<before , after> so when we want to do undo then draw the before instead of after
    std::vector<ImageObject *> undoActions, redoActions;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void import();
    void save();
    void insert_image(QImage image);
    void crop();
    void makeViewsAvailable();
    void makeViewsUnavailable();
    void close();
    void undo();
    void redo();
    void rotate(int value);
    void save_rotation_image();
    void zoom(int value);
    void save_zoomed_image();


protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

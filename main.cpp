#include "mainwindow.h"
#include <QApplication>
#include <bits/stdc++.h>


int main(int argc, char *argv[])
{

    std::cout << "hellllllllo";
    QApplication app(argc, argv);
    MainWindow widget;
    widget.showMaximized();
    //widget.show();
    return app.exec();
}

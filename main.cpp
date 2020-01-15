#include "glwidget.h"
#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>

#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTimer	timer;

    QMainWindow	window;
    QGridLayout layout;
    GLWidget w;
    QWidget	centralWidget,originalWidget;

    w.setFixedSize(1280, 720);

    window.setCentralWidget(&w);
    window.adjustSize();

    QObject::connect(&timer, SIGNAL(timeout()), &w, SLOT(updateFrame()));
    timer.start(50);

//    w.loadYUV();

	window.show();

    return a.exec();
}

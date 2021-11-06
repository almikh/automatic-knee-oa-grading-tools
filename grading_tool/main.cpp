#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

#include <opencv2/opencv.hpp>

// https://github.com/serizba/cppflow
// https://www.tensorflow.org/install/lang_c

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.init();
  w.show();

  return a.exec();
}

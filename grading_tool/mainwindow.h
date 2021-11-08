#pragma once
#include <QMainWindow>

#include <opencv2/opencv.hpp>
#include <tfdetect.h>

#include "viewport.h"
#include "classifier.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

protected:
  Viewport* viewport_;
  Classifier classifier_;

protected:
  void makeMenuFile();
  void makeToolbar();

  Q_SLOT void openSample(bool);

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void init();

  void runOnImage(const cv::Mat& frame);

  QPair<QString, float> runClassifier(const cv::Mat& joint_area);
};

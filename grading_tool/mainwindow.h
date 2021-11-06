#pragma once
#include <QMainWindow>

#include <opencv2/opencv.hpp>
#include <tfdetect.h>

#include "classifier.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

protected:
  Classifier classifier_;

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void init();

  void runOnImage(const cv::Mat& frame);
  void runClassifier(const cv::Mat& joint_area);
};

#pragma once
#include <QMainWindow>
#include <QVBoxLayout>
#include <QChartView>

#include <opencv2/opencv.hpp>
#include <tfdetect.h>

#include "viewport.h"
#include "classifier.h"

class QCustomPlot;

class MainWindow : public QMainWindow
{
  Q_OBJECT

protected:
  Viewport* viewport_;
  QVBoxLayout* right_panel_;
  Classifier classifier_;

protected:
  void makeMenuFile();
  void makeToolbar();

  QtCharts::QChartView* makeGraph(const QString& title, QColor color, const QVector<Classifier::Item>& data);
  
  Q_SLOT void openSample(bool);

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void init();

  void runOnImage(const cv::Mat& frame);

  QVector<Classifier::Item> runClassifier(const cv::Mat& joint_area);
};

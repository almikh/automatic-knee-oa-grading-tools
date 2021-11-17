#pragma once
#include <QMainWindow>
#include <QVBoxLayout>
#include <QChartView>

#include <opencv2/opencv.hpp>
#include <tfdetect.h>

#include "viewport.h"
#include "classifier.h"
#include "metadata.h"

class QCustomPlot;
class ViewQueue;

class MainWindow : public QMainWindow
{
  Q_OBJECT

protected:
  Viewport* viewport_;
  ViewQueue* view_queue_;
  QWidget* right_panel_;
  Classifier classifier_;

protected:
  void makeMenuFile();
  void makeToolbar();

  QtCharts::QChartView* makeGraph(const QString& title, QColor color, const QVector<Classifier::Item>& data);
  
  Q_SLOT void openSample(bool);
  Q_SLOT void runOnData(Metadata::HardPtr data);

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void init();

  QVector<Classifier::Item> runClassifier(const cv::Mat& joint_area);
};

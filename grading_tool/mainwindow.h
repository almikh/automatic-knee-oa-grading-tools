#pragma once
#include <QMainWindow>
#include <QVBoxLayout>
#include <QChartView>

#include <opencv2/opencv.hpp>

#include "tfdetect/tfdetect.h"
#include "viewport.h"
#include "classifier.h"
#include "metadata.h"

class QTableWidget;
class QCustomPlot;
class QStackedWidget;
class ProgressIndicator;
class ViewQueue;

namespace tfdetect {
  class Detector;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

protected:
  Viewport* viewport_;
  ViewQueue* view_queue_;
  QWidget* loading_area_;
  ProgressIndicator* loading_ind_;
  QStackedWidget* working_area_;
  QTableWidget* right_panel_;

  Metadata::HardPtr current_item_;
  std::shared_ptr<tfdetect::Detector> detector_;
  QSet<Metadata*> in_process_;
  Classifier classifier_;

protected:
  void makeMenuFile();
  void makeToolbar();

  QtCharts::QChartView* makeGraph(const QString& title, QColor color, const QVector<Classifier::Item>& data);
  
  Q_SLOT void openSample(bool);
  Q_SLOT void openDICOM(bool);

  // process (or display) specified item
  Q_SLOT void setItemAsCurrent(Metadata::HardPtr data); 

  // display all data of specified item
  Q_SLOT void showItem(Metadata::HardPtr data); 

  /// run models to localize joints and classify AO grades
  Q_SLOT void runOnData(Metadata::HardPtr data);

  Q_SIGNAL void itemProcessed(Metadata::HardPtr data);

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void init();

  QVector<Classifier::Item> runClassifier(const cv::Mat& joint_area);
};

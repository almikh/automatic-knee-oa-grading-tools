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
class QLabel;

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
  QLabel* calib_coef_;
  QWidget* working_area_;
  QTableWidget* right_panel_;

  QPushButton* zoom_menu_ = nullptr;
  QPushButton* draw_line_ = nullptr;
  QPushButton* draw_circle_ = nullptr;
  QPushButton* draw_angle_ = nullptr;
  QPushButton* draw_poly_ = nullptr;
  QPushButton* proc_menu_ = nullptr;
  QPushButton* transform_menu_ = nullptr;
  QAction* reset_calib_ = nullptr;

  Metadata::HardPtr current_item_;
  std::shared_ptr<tfdetect::Detector> detector_;
  QSet<Metadata*> in_process_;
  bool classifier_enabled_ = false;
  Classifier classifier_;

protected:
  void makeMenuFile();
  void makeMenuTools();
  void makeToolbar();

  QPushButton* createOptionButton(QIcon icon, bool enabled = true);
  QtCharts::QChartView* makeGraph(const QString& title, QColor color, const QVector<Classifier::Item>& data);

  void initClassifier();

  Q_SLOT void openSample(bool);
  Q_SLOT void openDICOM(bool);
  void open(const QString& filename, cv::Mat image);

  Q_SLOT void setCalibration();
  Q_SLOT void resetCalibration();
  Q_SLOT void showSettings();

  // process (or display) specified item
  Q_SLOT void setItemAsCurrent(Metadata::HardPtr data); 

  // update all data of current item
  Q_SLOT void updateCurrentItem(); 
  
  void saveCurrentGraphicsItems();

  Q_SLOT void onItemProcessed(Metadata::HardPtr data);

  // run models to localize joints and classify AO grades
  Q_SLOT void runOnData(Metadata::HardPtr data);

  // calibrate measuring units over selected item
  Q_SLOT void calibrate(GraphicsItem* item, const QPoint& pt);

  void rotateCurrentItem(int angle);
  void applyFilterForCurrent(cv::Mat filter, float delta = 0.0f, bool apply_to_gray = false);

  Q_SLOT void mousePosChanged(const QPoint&);
  Q_SLOT void mousePosOutOfImage();
  
  Q_SIGNAL void itemProcessed(Metadata::HardPtr data);

  Q_SLOT void drawLine();
  Q_SLOT void drawCircle();
  Q_SLOT void drawAngle();
  Q_SLOT void drawPoly();

  // zoom menu
  Q_SLOT void showZoomMenu();
  Q_SLOT void showProcMenu();
  Q_SLOT void showTransformMenu();
  Q_SLOT void fillViewport();
  Q_SLOT void zoomIn();
  Q_SLOT void zoomOut();
  Q_SLOT void zoom200();
  Q_SLOT void zoom400();
  Q_SLOT void zoom800();

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void init();

  QVector<Classifier::Item> runClassifier(const cv::Mat& joint_area);
};

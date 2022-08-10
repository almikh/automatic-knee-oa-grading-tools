#pragma once
#include <QMainWindow>
#include <QLinkedList>
#include <QVBoxLayout>
#include <QChartView>

#include <opencv2/opencv.hpp>

#include "tfdetect/tfdetect.h"
#include "viewport.h"
#include "classifier.h"
#include "metadata.h"
#include "tfdetect.h"

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
  static const QString ic_line;
  static const QString ic_zoom;
  static const QString ic_angle;
  static const QString ic_reset;
  static const QString ic_circle;
  static const QString ic_poly;
  static const QString ic_smart;
  static const QString ic_cobb_angle;
  static const QString ic_transform;
  static const QString ic_filter;
  
protected:
  Viewport* viewport_;
  ViewQueue* view_queue_;
  QWidget* loading_area_;
  ProgressIndicator* loading_ind_;
  QLabel* calib_coef_;
  QWidget* working_area_;
  QTableWidget* right_panel_;

  QPushButton* zoom_menu_ = nullptr;
  QPair<QPushButton*, QAction*> draw_line_ = { nullptr, nullptr };
  QPair<QPushButton*, QAction*> draw_circle_ = { nullptr, nullptr };
  QPair<QPushButton*, QAction*> draw_angle_ = { nullptr, nullptr };
  QPair<QPushButton*, QAction*> draw_cobb_angle_ = { nullptr, nullptr };
  QPair<QPushButton*, QAction*> draw_poly_ = { nullptr, nullptr };
  QPair<QPushButton*, QAction*> smart_curve_ = { nullptr, nullptr };
  QPushButton* proc_menu_ = nullptr;
  QPushButton* transform_menu_ = nullptr;
  QAction* reset_calib_ = nullptr;
  QAction* find_contours_ = nullptr;
  QAction* calibrate_ = nullptr;
  
  Metadata::HardPtr current_item_;
  QString detector_graph_file_ = "frozen_inference_graph.pb";
  std::shared_ptr<tfdetect::Detector> detector_;
  QLinkedList<Metadata::HardPtr> process_queue_;
  QSet<Metadata*> in_process_;
  bool classifier_enabled_ = false;
  Classifier classifier_;

protected:
  void makeMenuFile();
  void makeMenuMeasure();
  void makeMenuTools();
  void makeToolbar();

  QPushButton* createOptionButton(QIcon icon, bool enabled = true);
  QtCharts::QChartView* makeGraph(const QString& title, QColor color, const QVector<Classifier::Item>& data);

  void initClassifier();

  Q_SLOT void openSample(bool);
  Q_SLOT void openSamples(bool);
  Q_SLOT void openFolder(bool);

  Q_SLOT void openFileDICOM(bool);
  Q_SLOT void openFolderDICOM(bool);

  void open(const QString& filename, cv::Mat image);
  void openDICOM(const QString& path);

  Q_SLOT void setCalibration();
  Q_SLOT void resetCalibration();
  Q_SLOT void showSettings();

  // process (or display) specified item
  Q_SLOT void setItemAsCurrent(Metadata::HardPtr data); 

  // update all data of current item
  Q_SLOT void updateCurrentItem(); 
  
  void saveCurrentGraphicsItems();

  Q_SLOT void onItemProcessed(Metadata::HardPtr data);
  Q_SLOT void onContoursFound(const QVector<QVector<QPoint>>& contours);

  Q_SLOT void calibrateForLength(qreal length);

  // run models to localize joints and classify AO grades
  Q_SLOT void runOnData(Metadata::HardPtr data);

  // run models to localize joints and classify AO grades
  Q_SLOT void findContoursOnData(Metadata::HardPtr data);

  // calibrate measuring units over selected item
  Q_SLOT void calibrate(GraphicsItem* item, const QPoint& pt);

  void rotateCurrentItem(int angle);
  void applyFilterForCurrent(cv::Mat filter, float delta = 0.0f, bool apply_to_gray = false);

  Q_SLOT void mousePosChanged(const QPoint&);
  Q_SLOT void mousePosOutOfImage();
  
  Q_SIGNAL void itemProcessed(Metadata::HardPtr data);
  Q_SIGNAL void contoursFound(const QVector<QVector<QPoint>>& contours);

  Q_SLOT void drawLine(bool);
  Q_SLOT void drawCircle(bool);
  Q_SLOT void drawAngle(bool);
  Q_SLOT void drawCobbAngle(bool);
  Q_SLOT void smartCurve(bool);
  Q_SLOT void drawPoly(bool);
  void clearModeView();

  Q_SLOT void findContours(bool);

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
  std::vector<cv::Rect> runDetector(const cv::Mat& input_image);
};

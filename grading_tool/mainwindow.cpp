#include "mainwindow.h"
#include <QFileDialog>
#include <QMenuBar>
#include <QDebug>

#undef slots
#include <torch/script.h>
# define slots Q_SLOTS

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  viewport_(new Viewport())
{
  viewport_->setAutoscaleEnabled(false);
  setCentralWidget(viewport_);

  makeMenuFile();
  // makeToolbar();

  resize(800, 600);
}

MainWindow::~MainWindow() {
  
}

void MainWindow::makeMenuFile() {
  auto menu = menuBar()->addMenu("File");

  auto open_sample = menu->addAction("Open file");
  connect(open_sample, &QAction::triggered, this, &MainWindow::openSample);
}

void MainWindow::makeToolbar() {
  auto toolbar = addToolBar("main");
}

void MainWindow::init() {
  classifier_.initFromResource("D:\\Development\\automatic-knee-oa-grading-tools\\cnn_converter\\script.zip");
}

void MainWindow::runOnImage(const cv::Mat& sample) {
  auto detector = tfdetect::CreateDetectorFromGraph("frozen_inference_graph.pb");

  std::vector<tfdetect::Detection> results;
  detector->detect(sample, results);

  for (auto r : results) {
    auto rect = cv::Rect(r.x_min * sample.cols, r.y_min * sample.rows, (r.x_max - r.x_min) * sample.cols, (r.y_max - r.y_min) * sample.rows);

    auto [grade, conf] = runClassifier(sample(rect));

    cv::rectangle(sample, rect, cv::Scalar(0, 175, 0), 3);

    auto font_face = cv::FONT_HERSHEY_PLAIN;
    auto caption = grade.toStdString() + ": " + std::to_string(conf);
    auto sz = cv::getTextSize(caption, font_face, 1.5, 1, nullptr);
    cv::rectangle(sample, cv::Point(rect.x - 2, rect.tl().y - sz.height - 16), cv::Point(rect.x+ sz.width + 16, rect.tl().y), cv::Scalar(0, 175, 0), -1);
    cv::putText(sample, caption, cv::Point(rect.x, rect.tl().y - sz.height/2), font_face, 1.5, cv::Scalar::all(255));
  }

  viewport_->setImage(sample);
}

QPair<QString, float> MainWindow::runClassifier(const cv::Mat& joint_area) {
  cv::Mat working_image;
  cv::resize(joint_area, working_image, classifier_.getFrameSize());

  qDebug() << "";
  auto grades = classifier_.getGrade(working_image);
  for (auto grade : grades) {
    qDebug() << grade.mnemonic_code << grade.confidence;
  }

  return qMakePair(grades.front().mnemonic_code, grades.front().confidence);
}

void MainWindow::openSample(bool) {
  auto filters = "Image files (*.bmp *.png *.jpg *.jpeg);;";
  auto path = QFileDialog::getOpenFileName(this, "Load image", "", filters);
  if (!path.isEmpty()) {
    auto sample = cv::imread(path.toLocal8Bit().data(), cv::IMREAD_COLOR);
    runOnImage(sample);
  }
}
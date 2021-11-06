#include "mainwindow.h"
#include <QDebug>

#undef slots
#include <torch/script.h>
# define slots Q_SLOTS

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent) {
  resize(800, 600);
}

MainWindow::~MainWindow() {

}

void MainWindow::init() {
  classifier_.initFromResource("D:\\Development\\automatic-knee-oa-grading-tools\\cnn_converter\\script.zip");

  auto sample = cv::imread("9001695_00422803.jpg");
  runOnImage(sample);
}

void MainWindow::runOnImage(const cv::Mat& sample) {
  auto detector = tfdetect::CreateDetectorFromGraph("frozen_inference_graph.pb");

  std::vector<tfdetect::Detection> results;
  detector->detect(sample, results);

  for (auto r : results) {
    auto rect = cv::Rect(r.x_min * sample.cols, r.y_min * sample.rows, (r.x_max - r.x_min) * sample.cols, (r.y_max - r.y_min) * sample.rows);

    runClassifier(sample(rect));

    cv::rectangle(sample, rect, cv::Scalar(255, 0, 0), 3);
  }

  cv::imwrite("result.png", sample);
}

void MainWindow::runClassifier(const cv::Mat& joint_area) {
  cv::Mat working_image;
  cv::resize(joint_area, working_image, classifier_.getFrameSize());
  qDebug() << working_image.cols << working_image.rows << joint_area.channels();

  auto grades = classifier_.getGrade(working_image);
  for (auto grade : grades) {
    qDebug() << grade.mnemonic_code << grade.confidence;
  }
}

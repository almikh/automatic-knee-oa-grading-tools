#include "mainwindow.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QBarSet>
#include <QChart>
#include <QMenuBar>
#include <QDebug>

#undef slots
#include <torch/script.h>
# define slots Q_SLOTS

#include "qcustomplot.h"

using namespace QtCharts;

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  viewport_(new Viewport())
{
  auto w = new QWidget();
  auto l = new QHBoxLayout(w);

  viewport_->setAutoscaleEnabled(true);
  viewport_->setAutoscalePolicy(Viewport::AutoscalePolicy::MinFactor);
  l->addWidget(viewport_);

  auto right_panel = new QWidget();
  right_panel_ = new QVBoxLayout(right_panel);
  right_panel_->setAlignment(Qt::AlignTop);
  right_panel->setFixedWidth(250);
  l->addWidget(right_panel);

  makeMenuFile();
  // makeToolbar();

  setCentralWidget(w);
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

QChartView* MainWindow::makeGraph(const QString& title, QColor color, const QVector<Classifier::Item>& data) {
  QStringList categories;
  auto values = new QBarSet("Grade");
  values->setColor(color);
  values->setBorderColor(Qt::gray);
  for (int k = 0; k < data.size(); ++k) {
    categories.push_back(data[k].mnemonic_code);
    values->append(data[k].confidence);
  }

  auto series = new QBarSeries();
  series->append(values);

  auto chart = new QChart();
  chart->addSeries(series);
  chart->setAnimationOptions(QChart::SeriesAnimations);
  if (!title.isEmpty()) {
    chart->setTitle(title);
  }

  auto axisX = new QBarCategoryAxis();
  axisX->append(categories);
  chart->addAxis(axisX, Qt::AlignBottom);
  series->attachAxis(axisX);

  auto axisY = new QValueAxis();
  axisY->setRange(0, 1);
  chart->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisY);

  chart->legend()->setVisible(false);
  chart->legend()->setAlignment(Qt::AlignBottom);

  auto chart_view = new QChartView(chart);
  chart_view->setRenderHint(QPainter::Antialiasing);
  chart_view->setFixedHeight(200);
  return chart_view;
}

void MainWindow::init() {
  classifier_.initFromResource("D:\\Development\\automatic-knee-oa-grading-tools\\cnn_converter\\script.zip");
}

void MainWindow::runOnImage(const cv::Mat& sample) {
  auto detector = tfdetect::CreateDetectorFromGraph("frozen_inference_graph.pb");

  std::vector<tfdetect::Detection> results;
  detector->detect(sample, results);

  std::tuple<QString, QColor, cv::Scalar> colors[] = {
    {QString("Red Area"), QColor(Qt::red), cv::Scalar(200, 0, 0)},
    {QString("Blue Area"), QColor(Qt::blue), cv::Scalar(0, 0, 200)},
  };

  for (int k = 0; k< static_cast<int>(results.size()); ++k) {
    auto r = results[k];
    auto rect = cv::Rect(
      r.x_min * sample.cols, 
      r.y_min * sample.rows, 
      (r.x_max - r.x_min) * sample.cols, 
      (r.y_max - r.y_min) * sample.rows);

    auto grades = runClassifier(sample(rect));

    int grade_idx = 0;
    for (int k = 1; k < grades.size(); ++k) {
      if (grades[k].confidence > grades[grade_idx].confidence) {
        grade_idx = k;
      }
    }

    auto grade = grades[grade_idx].mnemonic_code;
    auto conf = grades[grade_idx].confidence;

    cv::rectangle(sample, rect, std::get<2>(colors[k]), 3);

    auto title = QString("Grade %1, %2").arg(grade).arg(conf);
    right_panel_->addWidget(makeGraph(title, std::get<1>(colors[k]), grades));
  }
  
  viewport_->setImage(sample);
}

QVector<Classifier::Item> MainWindow::runClassifier(const cv::Mat& joint_area) {
  cv::Mat working_image;
  cv::resize(joint_area, working_image, classifier_.getFrameSize());

  qDebug() << "";
  auto grades = classifier_.getGrade(working_image);
  for (auto grade : grades) {
    qDebug() << grade.mnemonic_code << grade.confidence;
  }

  return grades;
}

void MainWindow::openSample(bool) {
  auto filters = "Image files (*.bmp *.png *.jpg *.jpeg);;";
  auto path = QFileDialog::getOpenFileName(this, "Load image", "", filters);
  if (!path.isEmpty()) {
    auto sample = cv::imread(path.toLocal8Bit().data(), cv::IMREAD_COLOR);
    runOnImage(sample);
  }
}
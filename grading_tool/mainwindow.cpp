#include "mainwindow.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QGraphicsLayout>
#include <QValueAxis>
#include <QBarSet>
#include <QChart>
#include <QSplitter>
#include <QMenuBar>
#include <QDebug>

#undef slots
#include <torch/script.h>
# define slots Q_SLOTS

#include "utils.h"
#include "view_queue.h"

using namespace QtCharts;

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  viewport_(new Viewport()),
  view_queue_(new ViewQueue())
{
  auto splitter = new QSplitter();
  splitter->setOrientation(Qt::Vertical);

  auto w = new QWidget();
  auto l = new QHBoxLayout(w);
  viewport_->setAutoscaleEnabled(true);
  viewport_->setAutoscalePolicy(Viewport::AutoscalePolicy::MinFactor);
  l->addWidget(viewport_);

  right_panel_ = new QWidget();
  right_panel_->setFixedWidth(250);
  l->addWidget(right_panel_);
  splitter->addWidget(w);

  view_queue_->setMinimumHeight(100);
  view_queue_->setMaximumHeight(225);
  connect(view_queue_, &ViewQueue::currentItemChanged, this, &MainWindow::runOnData);
  splitter->addWidget(view_queue_);

  for (int k = 0; k < splitter->count(); ++k) {
    splitter->setCollapsible(k, false);
  }

  makeMenuFile();
  // makeToolbar();

  setCentralWidget(splitter);
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

  chart->legend()->hide();
  chart->layout()->setContentsMargins(0, 0, 0, 0);
  chart->setAcceptHoverEvents(true);

  auto chart_view = new QChartView(chart);
  chart_view->setRenderHint(QPainter::Antialiasing);
  chart_view->setMaximumHeight(200);
  return chart_view;
}

void MainWindow::init() {
  classifier_.initFromResource("D:\\Development\\automatic-knee-oa-grading-tools\\cnn_converter\\script.zip");
}

void MainWindow::runOnData(Metadata::HardPtr data) {
  auto sample = data->image;
  auto detector = tfdetect::CreateDetectorFromGraph("frozen_inference_graph.pb");

  std::vector<tfdetect::Detection> knee_joints;
  detector->detect(sample, knee_joints);

  std::tuple<QString, QColor, cv::Scalar> colors[] = {
    {QString("Red Area"), QColor(Qt::red), cv::Scalar(200, 0, 0)},
    {QString("Blue Area"), QColor(Qt::blue), cv::Scalar(0, 0, 200)},
  };

  // remove previous graphs
  if (auto layout = right_panel_->layout()) {
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != 0) {
      layout->removeItem(item);
    }

    delete layout;
  }
    
  auto graphs = new QVBoxLayout(right_panel_);
  graphs->setAlignment(Qt::AlignTop);

  // classification
  for (int k = 0; k< static_cast<int>(knee_joints.size()); ++k) {
    auto r = knee_joints[k];
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
    graphs->addWidget(makeGraph(title, std::get<1>(colors[k]), grades));
    graphs->setStretch(graphs->count() - 1, 1);
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
    auto filename = QFileInfo(path).fileName();
    auto sample = cv::imread(path.toLocal8Bit().data(), cv::IMREAD_COLOR);

    auto item = std::make_shared<Metadata>();
    item->filename = filename;
    item->image = sample;

    view_queue_->addItem(item);
  }
}

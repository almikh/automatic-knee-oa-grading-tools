#include "mainwindow.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QGraphicsLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QValueAxis>
#include <QtConcurrent>
#include <QBarSet>
#include <QChart>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidget>
#include <QToolBar>
#include <QSplitter>
#include <QMenuBar>
#include <QDebug>

#undef slots
#include <torch/script.h>
#define slots Q_SLOTS

#include <gdcm/gdcmImageReader.h>

#include "utils.h"
#include "view_queue.h"
#include "progress_indicator.h"

using namespace QtCharts;

QAction* nameAction(QToolBar* bar, QAction* action, const char* name = nullptr) {
  if (name && action->objectName().isEmpty())
    action->setObjectName(name);

  bar->widgetForAction(action)->setObjectName(action->objectName());
  return action;
}

const std::tuple<QString, QColor, cv::Scalar> joint_colors[] = {
  {QString("Red Area"), QColor(Qt::red), cv::Scalar(200, 0, 0)},
  {QString("Blue Area"), QColor(Qt::blue), cv::Scalar(0, 0, 200)},
};

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  viewport_(new Viewport()),
  loading_area_(new QWidget()),
  loading_ind_(new ProgressIndicator()),
  working_area_(new QStackedWidget()),
  right_panel_(new QTableWidget()),
  view_queue_(new ViewQueue())
{
  setWindowTitle("Osteoarthritis Grading Tool");

  auto splitter = new QSplitter();
  splitter->setOrientation(Qt::Vertical);

  auto w = new QWidget();
  auto layout = new QHBoxLayout(w);

  // sample's viewport
  working_area_->addWidget(viewport_);

  // reset viewport state
  auto ll = new QHBoxLayout(viewport_);
  auto reset = createOptionButton(QIcon(":/ic_reset"));
  ll->addWidget(reset, 0, Qt::AlignTop | Qt::AlignLeft);

  zoom_menu_ = createOptionButton(QIcon(":/ic_zoom"), false);
  ll->addWidget(zoom_menu_, 0, Qt::AlignTop | Qt::AlignLeft);

  draw_line_ = createOptionButton(QIcon(":/ic_line"));
  ll->addWidget(draw_line_, 0, Qt::AlignTop | Qt::AlignLeft);

  draw_circle_ = createOptionButton(QIcon(":/ic_circle"));
  ll->addWidget(draw_circle_, 0, Qt::AlignTop | Qt::AlignLeft);

  draw_angle_ = createOptionButton(QIcon(":/ic_angle"));
  ll->addWidget(draw_angle_, 0, Qt::AlignTop | Qt::AlignLeft);

  ll->addWidget(new QWidget(), 1, Qt::AlignTop | Qt::AlignLeft);

  connect(reset, &QPushButton::clicked, viewport_, &Viewport::fitImageToViewport);
  connect(zoom_menu_, &QPushButton::clicked, this, &MainWindow::showZoomMenu);
  connect(draw_line_, &QPushButton::clicked, this, &MainWindow::drawLine);
  connect(draw_circle_, &QPushButton::clicked, this, &MainWindow::drawCircle);
  connect(draw_angle_, &QPushButton::clicked, this, &MainWindow::drawAngle);
  
  // loading area
  working_area_->addWidget(loading_area_);
  working_area_->setStyleSheet("background-color: black");

  auto l = new QVBoxLayout(loading_area_); 
  loading_ind_->setFixedSize(150, 150);
  loading_ind_->setColor(Qt::white);
  l->addWidget(loading_ind_, 0, Qt::AlignCenter);

  layout->addWidget(working_area_);

  // graphs area
  right_panel_->setShowGrid(false);
  right_panel_->verticalHeader()->hide();
  right_panel_->horizontalHeader()->hide();
  right_panel_->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  right_panel_->setSelectionMode(QAbstractItemView::NoSelection);
  right_panel_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
  right_panel_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
  right_panel_->setColumnCount(1);
  right_panel_->setFixedWidth(200);
  layout->addWidget(right_panel_);
  splitter->addWidget(w);

  view_queue_->setMinimumHeight(100);
  view_queue_->setMaximumHeight(225);
  connect(view_queue_, &ViewQueue::currentItemChanged, this, &MainWindow::setItemAsCurrent);
  splitter->addWidget(view_queue_);

  for (int k = 0; k < splitter->count(); ++k) {
    splitter->setCollapsible(k, false);
  }

  makeMenuFile();
  // makeToolbar();

  connect(this, &MainWindow::itemProcessed, this, &MainWindow::showItem, Qt::QueuedConnection);
  connect(viewport_, &Viewport::mousePosChanged, this, &MainWindow::mousePosChanged);
  connect(viewport_, &Viewport::mousePosOutOfImage, this, &MainWindow::mousePosOutOfImage);
  connect(viewport_, &Viewport::menuForItemRequested, [=](GraphicsItem* item, const QPoint& pt) {
    auto menu = new QMenu();

    if (item->getType() == GraphicsItem::Type::Line) {
      auto calibrate = new QAction("Calibrate", menu);
      calibrate->setStatusTip("Unit Calibration");
      connect(calibrate, &QAction::triggered, [=]() {
        this->calibrate(item, pt);
      });
      menu->addAction(calibrate);
    }

    auto remove = new QAction("Remove", menu);
    remove->setStatusTip("Remove selected graphics item");
    connect(remove, &QAction::triggered, [=]() {
      viewport_->removeGraphicsItem(item);
    });
    menu->addAction(remove);

    menu->exec(viewport_->mapToGlobal(pt));
  });
  
  setCentralWidget(splitter);
  resize(800, 600);
}

MainWindow::~MainWindow() {
  
}

void MainWindow::makeMenuFile() {
  auto menu = menuBar()->addMenu("File");

  auto open_sample = menu->addAction("Open file");
  connect(open_sample, &QAction::triggered, this, &MainWindow::openSample);

  auto open_dicom = menu->addAction("Open DICOM");
  connect(open_dicom, &QAction::triggered, this, &MainWindow::openDICOM);
}

void MainWindow::showZoomMenu() {
  auto menu = new QMenu();

  auto fill_viewport = new QAction("Fill viewport", menu);
  fill_viewport->setShortcut(QKeySequence("CTRL+0"));
  fill_viewport->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  fill_viewport->setStatusTip("Scale the current image to the viewport size");
  connect(fill_viewport, &QAction::triggered, this, &MainWindow::fillViewport);

  menu->addAction(fill_viewport);
  menu->addSeparator();

  auto zoom200 = new QAction("200%", menu);
  auto zoom400 = new QAction("400%", menu);
  auto zoom800 = new QAction("800%", menu);
  zoom200->setShortcut(QKeySequence("CTRL+2"));
  zoom400->setShortcut(QKeySequence("CTRL+3"));
  zoom800->setShortcut(QKeySequence("CTRL+4"));
  connect(zoom200, &QAction::triggered, this, &MainWindow::zoom200);
  connect(zoom400, &QAction::triggered, this, &MainWindow::zoom400);
  connect(zoom800, &QAction::triggered, this, &MainWindow::zoom800);

  menu->addAction(zoom200);
  menu->addAction(zoom400);
  menu->addAction(zoom800);
  menu->addSeparator();

  auto zoom_in = new QAction("Zoom in", menu);
  auto zoom_out = new QAction("Zoom out", menu);
  zoom_in->setShortcut(QKeySequence("CTRL + +"));
  zoom_out->setShortcut(QKeySequence("CTRL + -"));
  connect(zoom_in, &QAction::triggered, this, &MainWindow::zoomIn);
  connect(zoom_out, &QAction::triggered, this, &MainWindow::zoomOut);

  menu->addAction(zoom_in);
  menu->addAction(zoom_out);

  menu->setShortcutEnabled(0, true);
  menu->exec(zoom_menu_->mapToGlobal(QPoint(0, 28)));
}

void MainWindow::makeToolbar() {
  auto toolbar = addToolBar("main");

  auto line = nameAction(toolbar, toolbar->addAction(QIcon(":/ic_line"), "Draw Line"), "line");
  auto calibrate = nameAction(toolbar, toolbar->addAction(QIcon(":/ic_calibrate"), "Calibrate"), "calibrate");
  toolbar->setStyleSheet(
    "QToolButton#calibrate { background:green }"
    "QToolButton#line { background:blue }");

  Q_UNUSED(calibrate);
  Q_UNUSED(line);
}

QPushButton* MainWindow::createOptionButton(QIcon icon, bool enabled) {
  auto button = new QPushButton(icon, "");
  button->setFocusPolicy(Qt::NoFocus);
  button->setFixedSize(24, 24);
  button->setCheckable(true);
  button->setChecked(false);
  button->setEnabled(enabled);

  button->setStyleSheet(
    "QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } "
    "QPushButton:!enabled { border-width: 1px; border-style: outset; border-color: black; background-color: gray;");

  return button;
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
  chart_view->setAlignment(Qt::AlignCenter);
  chart_view->setRenderHint(QPainter::Antialiasing);
  chart_view->setMaximumHeight(200);
  chart_view->setMaximumWidth(200);
  return chart_view;
}

void MainWindow::init() {
  classifier_.initFromResource("D:\\Development\\automatic-knee-oa-grading-tools\\cnn_converter\\script.zip");
}

void MainWindow::setItemAsCurrent(Metadata::HardPtr data) {
  if (!current_item_) {
    current_item_ = data;
  }

  if (current_item_ && current_item_ != data) {
    current_item_->viewport_state = viewport_->state();
  }

  // is already calculating - just wait for it
  if (in_process_.contains(data.get())) {
    current_item_ = data;
    working_area_->setCurrentWidget(loading_area_);
    loading_ind_->startAnimation();
    return;
  }

  if (data->joints.isEmpty()) {
    working_area_->setCurrentWidget(loading_area_);
    loading_ind_->startAnimation();
    in_process_.insert(data.get());
    current_item_ = data;

    QtConcurrent::run(this, &MainWindow::runOnData, data);
  }
  else {
    showItem(data);
  }
}

void MainWindow::showItem(Metadata::HardPtr data) {
  auto sample = data->image.clone();

  // remove previous graphs
  right_panel_->setRowCount(0);
  right_panel_->setRowCount(data->joints.size());
  for (int k = 0; k < data->joints.size(); ++k) {
    const auto& joint = data->joints[k];

    // calc maximum
    int grade_idx = 0;
    for (int i = 1; i < joint.grades.size(); ++i) {
      if (joint.grades[i].confidence > joint.grades[grade_idx].confidence) {
        grade_idx = i;
      }
    }

    // draw ROI
    cv::rectangle(sample, joint.rect, std::get<2>(joint_colors[k % 2]), 3);

    // create graph
    auto conf = joint.grades[grade_idx].confidence;
    auto grade = joint.grades[grade_idx].mnemonic_code;
    auto title = QString("Grade %1, %2").arg(grade).arg(conf);

    auto graph = makeGraph(title, std::get<1>(joint_colors[k % 2]), joint.grades);
    right_panel_->setCellWidget(k, 0, graph);
  }

  // previous item
  if (current_item_) {
    current_item_->viewport_state = viewport_->state();
  }

  // currentn item
  current_item_ = data;

  loading_ind_->stopAnimation();
  working_area_->setCurrentWidget(viewport_);
  viewport_->setImage(sample);

  // set actual image position and scale
  if (!current_item_->already_display) {
    current_item_->already_display = true;
  }
  else {
    viewport_->setState(current_item_->viewport_state);
  }

  view_queue_->updateView();
  zoom_menu_->setEnabled(true);
}

void MainWindow::runOnData(Metadata::HardPtr data) {
  auto sample = data->image.clone();

  // init detector
  if (!detector_) {
    detector_ = tfdetect::CreateDetectorFromGraph("frozen_inference_graph.pb");
  }

  // detect joints
  auto knee_joints = detector_->detect(sample);

  // classification
  for (int k = 0; k < static_cast<int>(knee_joints.size()); ++k) {
    auto r = knee_joints[k];
    auto rect = cv::Rect(
      r.x_min * sample.cols,
      r.y_min * sample.rows,
      (r.x_max - r.x_min) * sample.cols,
      (r.y_max - r.y_min) * sample.rows);

    auto grades = runClassifier(sample(rect));

    Metadata::Joint joint;
    joint.grades = grades;
    joint.rect = rect;
    data->joints.push_back(joint);
  }

  in_process_.remove(data.get());

  // if selected other item - just store processing results
  if (current_item_ == data) {
    emit itemProcessed(data);
  }
}

void MainWindow::calibrate(GraphicsItem* item, const QPoint& pt) {
  bool ok;
  auto desc = "Enter new distance for current item:";
  auto new_length = QInputDialog::getDouble(this, "Length calibration", desc, 0.0, 0.0, DBL_MAX, 2, &ok);
  if (ok) {
    auto coef = new_length / item->length();
    viewport_->setCalibrationCoef(coef);
  }
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

void MainWindow::openDICOM(bool) {
  auto filters = "DICOM files (*.DICOM *.DCM);;";
  auto path = QFileDialog::getOpenFileName(this, "Load DICOM file", "", filters);
  if (!path.isEmpty()) {
    gdcm::ImageReader reader;
    reader.SetFileName(path.toStdString().c_str());
    if (reader.Read()) {
      auto filename = QFileInfo(path).fileName();
      auto image = reader.GetImage();

      std::vector<char> buffer(image.GetBufferLength());
      image.GetBuffer(buffer.data());
      
      int cols = image.GetColumns();
      int rows = image.GetRows();

      int pixel_type;
      auto pixel_format = image.GetPixelFormat().GetScalarType();
      if (pixel_format == gdcm::PixelFormat::UINT8) pixel_type = CV_8UC1;
      else if (pixel_format == gdcm::PixelFormat::INT8) pixel_type = CV_8SC1;
      else if (pixel_format == gdcm::PixelFormat::UINT16) pixel_type = CV_16UC1;
      else if (pixel_format == gdcm::PixelFormat::INT16) pixel_type = CV_16SC1;
      else if (pixel_format == gdcm::PixelFormat::FLOAT32) pixel_type = CV_32FC1;
      else if (pixel_format == gdcm::PixelFormat::FLOAT64) pixel_type = CV_64FC1;
      else if (pixel_format == gdcm::PixelFormat::UINT32) pixel_type = CV_8UC4;
      else if (pixel_format == gdcm::PixelFormat::INT32) pixel_type = CV_8SC4;
      else {
        QMessageBox::warning(this, "Warning", "Unknown pixel format of image in DICOM!");
        return;
      }

      cv::Mat img(image.GetRows(), image.GetColumns(), pixel_type, buffer.data());

      auto item = std::make_shared<Metadata>();
      cv::cvtColor(img, item->image, cv::COLOR_GRAY2RGB);
      item->filename = filename;

      view_queue_->addItem(item);
    }
  }
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

void MainWindow::mousePosChanged(const QPoint& pt) {
  if (current_item_) {
    if (0 <= pt.x() && pt.x() < current_item_->image.cols && 0 <= pt.y() && pt.y() < current_item_->image.rows) {
      auto val = current_item_->image.at<cv::Vec3b>(pt.y(), pt.x());
      viewport_->setLabelText(QString("X: %1 Y: %2 Val: %3").arg(pt.x()).arg(pt.y()).arg(val[0]));
    }
  }
}

void MainWindow::mousePosOutOfImage() {
  if (current_item_) {
    viewport_->setLabelVisible(false);
  }
}

void MainWindow::drawLine() {
  if (viewport_->mode() != Viewport::Mode::DrawLine) {
    viewport_->setMode(Viewport::Mode::DrawLine);
    draw_line_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: rgb(28, 244, 19); } ");
    draw_circle_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
    draw_angle_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
  }
  else {
    viewport_->setMode(Viewport::Mode::View);
    draw_line_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
  }
}

void MainWindow::drawCircle() {
  if (viewport_->mode() != Viewport::Mode::DrawCircle) {
    viewport_->setMode(Viewport::Mode::DrawCircle);
    draw_circle_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: rgb(28, 244, 19); } ");
    draw_angle_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
    draw_line_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
  }
  else {
    viewport_->setMode(Viewport::Mode::View);
    draw_circle_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
  }
}

void MainWindow::drawAngle() {
  if (viewport_->mode() != Viewport::Mode::DrawAngle) {
    viewport_->setMode(Viewport::Mode::DrawAngle);
    draw_angle_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: rgb(28, 244, 19); } ");
    draw_circle_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
    draw_line_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
  }
  else {
    viewport_->setMode(Viewport::Mode::View);
    draw_angle_->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
  }
}

void MainWindow::fillViewport() {
  viewport_->fitImageToViewport();
  current_item_->viewport_state = viewport_->state();
}

void MainWindow::zoomIn() {
  viewport_->scaleBy(1.1);
  current_item_->viewport_state = viewport_->state();
}

void MainWindow::zoomOut() {
  viewport_->scaleBy(0.9);
  current_item_->viewport_state = viewport_->state();
}

void MainWindow::zoom200() {
  viewport_->scaleTo(2.0);
  current_item_->viewport_state = viewport_->state();
}

void MainWindow::zoom400() {
  viewport_->scaleTo(4.0);
  current_item_->viewport_state = viewport_->state();
}

void MainWindow::zoom800() {
  viewport_->scaleTo(8.0);
  current_item_->viewport_state = viewport_->state();
}

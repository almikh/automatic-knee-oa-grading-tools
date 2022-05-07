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
#include <QLabel>
#include <QDebug>

#undef slots
#include <torch/script.h>
#define slots Q_SLOTS

#include <gdcm/gdcmImageReader.h>

#include "utils.h"
#include "view_queue.h"
#include "progress_indicator.h"
#include "settings_window.h"
#include "app_preferences.h"
#include "defs.h"

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
  working_area_(new QWidget()),
  calib_coef_(new QLabel()),
  loading_ind_(new ProgressIndicator()),
  right_panel_(new QTableWidget()),
  view_queue_(new ViewQueue())
{
  setWindowTitle("Osteoarthritis Grading Tool");

  auto splitter = new QSplitter();
  splitter->setOrientation(Qt::Vertical);

  auto w = new QWidget();
  auto layout = new QHBoxLayout(w);

  // working area
  working_area_->setStyleSheet("background-color: black");
  auto wl = new QVBoxLayout(working_area_);
  wl->setContentsMargins(0, 0, 0, 0);

  // sample's viewport
  wl->addWidget(viewport_, 1);

  // loading area
  loading_area_->setFixedHeight(20);
  wl->addWidget(loading_area_, 0, Qt::AlignLeft);
  
  calib_coef_->setStyleSheet("background-color: black; color:white");

  auto l = new QHBoxLayout(loading_area_); 
  l->setContentsMargins(4, 1, 4, 1);

  loading_ind_->setFixedSize(16, 16);
  loading_ind_->setColor(Qt::white);
  l->addWidget(loading_ind_, 0, Qt::AlignLeft);
  l->addWidget(calib_coef_, 0, Qt::AlignLeft);
  
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

  makeToolbar();

  makeMenuFile();
  makeMenuMeasure();
  makeMenuTools();

  connect(this, &MainWindow::itemProcessed, this, &MainWindow::onItemProcessed, Qt::QueuedConnection);
  connect(viewport_, &Viewport::calibFinished, this, &MainWindow::calibrateForLength);
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
  AppPrefs::write("window/position", this->pos());
  AppPrefs::write("window/size", this->size());
}

void MainWindow::makeMenuFile() {
  auto menu = menuBar()->addMenu("File");

  auto open_sample = menu->addAction("Open file");
  connect(open_sample, &QAction::triggered, this, &MainWindow::openSample);

  auto open_samples = menu->addAction("Open files...");
  connect(open_samples, &QAction::triggered, this, &MainWindow::openSamples);


  menu->addSeparator();
  auto open_dicom = menu->addAction("Open DICOM");
  connect(open_dicom, &QAction::triggered, this, &MainWindow::openDICOM);
}

void MainWindow::makeMenuTools() {
  auto menu = menuBar()->addMenu("Tools");

  auto open_sample = menu->addAction("Options...");
  connect(open_sample, &QAction::triggered, this, &MainWindow::showSettings);
}

void MainWindow::makeMenuMeasure() {
  auto menu = menuBar()->addMenu("Measure and annotate");

  draw_line_.second = menu->addAction(QIcon(":/ic_line"), "Line");
  connect(draw_line_.second, &QAction::triggered, this, &MainWindow::drawLine);
  draw_line_.second->setCheckable(true);

  draw_angle_.second = menu->addAction(QIcon(":/ic_angle"), "Angle");
  connect(draw_angle_.second, &QAction::triggered, this, &MainWindow::drawAngle);
  draw_angle_.second->setCheckable(true);

  draw_cobb_angle_.second = menu->addAction(QIcon(":/ic_cobb_angle"), "Cobb Angle");
  connect(draw_cobb_angle_.second, &QAction::triggered, this, &MainWindow::drawCobbAngle);
  draw_cobb_angle_.second->setCheckable(true);

  draw_circle_.second = menu->addAction(QIcon(":/ic_circle"), "Ellipse");
  connect(draw_circle_.second, &QAction::triggered, this, &MainWindow::drawCircle);
  draw_circle_.second->setCheckable(true);

  draw_poly_.second = menu->addAction(QIcon(":/ic_poly"), "Closed polygon");
  connect(draw_poly_.second, &QAction::triggered, this, &MainWindow::drawPoly);
  draw_poly_.second->setCheckable(true);

  smart_curve_.second = menu->addAction(QIcon(":/ic_smart"), "Smart curve");
  connect(smart_curve_.second, &QAction::triggered, this, &MainWindow::smartCurve);
  smart_curve_.second->setCheckable(true);
  
  menu->addSeparator();

  calibrate_ = menu->addAction("Calibration");
  connect(calibrate_, &QAction::triggered, this, &MainWindow::setCalibration);
  calibrate_->setEnabled(false);

  reset_calib_ = menu->addAction("Reset Calibration");
  connect(reset_calib_, &QAction::triggered, this, &MainWindow::resetCalibration);
  reset_calib_->setEnabled(false);
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

void MainWindow::showTransformMenu() {
  auto menu = new QMenu();

  auto rotate_90_cw = new QAction("Rotate 90° CW", menu);
  connect(rotate_90_cw, &QAction::triggered, [this]() {
    rotateCurrentItem(Rotation::Rotate90_CW);
  });

  auto rotate_90_ccw = new QAction("Rotate 90° CCW", menu);
  connect(rotate_90_ccw, &QAction::triggered, [this]() {
    rotateCurrentItem(Rotation::Rotate90_CCW);
  });

  auto rotate_180 = new QAction("Rotate 180°", menu);
  connect(rotate_180, &QAction::triggered, [this]() {
    rotateCurrentItem(Rotation::Rotate180);
  });

  auto flip_h = new QAction("Flip horizontal", menu);
  connect(flip_h, &QAction::triggered, [this]() {
    if (current_item_->transformations.contains(Transformation::HFlip)) {
      current_item_->transformations.removeOne(Transformation::HFlip);
    }
    else {
      current_item_->transformations.push_back(Transformation::HFlip);
    }

    saveCurrentGraphicsItems();
    updateCurrentItem();
    clearModeView();
  });

  auto flip_v = new QAction("Flip vertical", menu);
  connect(flip_v, &QAction::triggered, [this]() {
    if (current_item_->transformations.contains(Transformation::VFlip)) {
      current_item_->transformations.removeOne(Transformation::VFlip);
    }
    else {
      current_item_->transformations.push_back(Transformation::VFlip);
    }

    saveCurrentGraphicsItems();
    updateCurrentItem();
    clearModeView();
  });

  auto clear = new QAction("Clear transformations", menu);
  connect(clear, &QAction::triggered, [this]() {
    if (!current_item_->transformations.isEmpty() || current_item_->rotation) {
      saveCurrentGraphicsItems();
      current_item_->transformations.clear();
      current_item_->rotation = 0;
      updateCurrentItem();
    }
  });

  menu->addAction(rotate_90_cw);
  menu->addAction(rotate_90_ccw);
  menu->addAction(rotate_180);
  menu->addSeparator();
  menu->addAction(flip_h);
  menu->addAction(flip_v);
  menu->addSeparator();
  menu->addAction(clear);

  menu->setShortcutEnabled(0, true);
  menu->exec(transform_menu_->mapToGlobal(QPoint(0, 28)));
}

void MainWindow::showProcMenu() {
  auto menu = new QMenu();

  auto none = new QAction("None", menu);
  connect(none, &QAction::triggered, [this]() {
    current_item_->image = current_item_->src_image.clone();
    updateCurrentItem();
  });

  auto inv = new QAction("Invert", menu);
  connect(inv, &QAction::triggered, [this]() {
    cv::Mat tmp;
    cv::cvtColor(current_item_->src_image, tmp, cv::COLOR_RGB2GRAY);
    tmp = 255 - tmp;
    cv::cvtColor(tmp, current_item_->image, cv::COLOR_GRAY2RGB);
    updateCurrentItem();
  });

  auto sharpen_1 = new QAction("Sharpen 1", menu);
  connect(sharpen_1, &QAction::triggered, [this]() {
    applyFilterForCurrent((cv::Mat_<double>(3, 3) <<
      0, -1, 0,
      -1, 5, -1,
      0, -1, 0));
  });

  auto sharpen_2 = new QAction("Sharpen 2", menu);
  connect(sharpen_2, &QAction::triggered, [this]() {
    applyFilterForCurrent((cv::Mat_<double>(3, 3) <<
      -1, -1, -1,
      -1, 9, -1,
      -1, -1, -1));
  });

  auto sharpen_3 = new QAction("Sharpen 3", menu);
  connect(sharpen_3, &QAction::triggered, [this]() {
    if (current_item_) {
      cv::Mat blurred;
      double sigma = 1, threshold = 5, amount = 1;
      cv::GaussianBlur(current_item_->image, blurred, cv::Size(), sigma, sigma);
      cv::Mat lowContrastMask = abs(current_item_->image - blurred) < threshold;
      cv::Mat sharpened = current_item_->image * (1 + amount) + blurred * (-amount);
      current_item_->image.copyTo(sharpened, lowContrastMask);
      current_item_->image = sharpened;
      updateCurrentItem();
    }
  });

  auto edge = new QAction("Edge 1", menu);
  connect(edge, &QAction::triggered, [this]() {
    applyFilterForCurrent((cv::Mat_<double>(3, 3) <<
      0, -1, 0,
      -1, 4, -1,
      0, -1, 0), 128.0f, true);
  });

  auto edge_2 = new QAction("Edge 2", menu);
  connect(edge_2, &QAction::triggered, [this]() {
    applyFilterForCurrent((cv::Mat_<double>(3, 3) <<
      -1, -2, -1,
      -2, 12, -2,
      -1, -2, -1), 128.0f, true);
  });

  auto emboss_n = new QAction("Emboss N", menu);
  connect(emboss_n, &QAction::triggered, [this]() {
    applyFilterForCurrent((cv::Mat_<double>(3, 3) <<
      1, 1, 1,
      0, 0, 0,
      -1, -1, -1), 128.0f, true);
  });

  auto emboss_w = new QAction("Emboss W", menu);
  connect(emboss_w, &QAction::triggered, [this]() {
    applyFilterForCurrent((cv::Mat_<double>(3, 3) <<
      1, 0, -1,
      1, 0, -1,
      1, 0, -1), 128.0f, true);
  });

  auto emboss_d = new QAction("Emboss D", menu);
  connect(emboss_d, &QAction::triggered, [this]() {
    applyFilterForCurrent((cv::Mat_<double>(3, 3) <<
       0, 1, 1,
      -1, 0, 1,
      -1, -1, 0), 128.0f, true);
  });

  menu->addAction(none);
  menu->addSeparator();
  menu->addAction(inv);
  menu->addSeparator();
  menu->addAction(sharpen_1);
  menu->addAction(sharpen_2);
  menu->addAction(sharpen_3);
  menu->addAction(edge);
  menu->addAction(edge_2);
  menu->addAction(emboss_n);
  menu->addAction(emboss_w);
  menu->addAction(emboss_d);

  menu->setShortcutEnabled(0, true);
  menu->exec(proc_menu_->mapToGlobal(QPoint(0, 28)));
}

void MainWindow::makeToolbar() {
  auto ll = new QHBoxLayout(viewport_);
  auto reset = createOptionButton(QIcon(":/ic_reset"));
  ll->addWidget(reset, 0, Qt::AlignTop | Qt::AlignLeft);

  zoom_menu_ = createOptionButton(QIcon(":/ic_zoom"), false);
  ll->addWidget(zoom_menu_, 0, Qt::AlignTop | Qt::AlignLeft);

  draw_line_.first = createOptionButton(QIcon(":/ic_line"));
  ll->addWidget(draw_line_.first, 0, Qt::AlignTop | Qt::AlignLeft);

  draw_angle_.first = createOptionButton(QIcon(":/ic_angle"));
  ll->addWidget(draw_angle_.first, 0, Qt::AlignTop | Qt::AlignLeft);

  draw_cobb_angle_.first = createOptionButton(QIcon(":/ic_cobb_angle"));
  ll->addWidget(draw_cobb_angle_.first, 0, Qt::AlignTop | Qt::AlignLeft);

  draw_circle_.first = createOptionButton(QIcon(":/ic_circle"));
  ll->addWidget(draw_circle_.first, 0, Qt::AlignTop | Qt::AlignLeft);

  draw_poly_.first = createOptionButton(QIcon(":/ic_poly"));
  ll->addWidget(draw_poly_.first, 0, Qt::AlignTop | Qt::AlignLeft);

  smart_curve_.first = createOptionButton(QIcon(":/ic_smart"));
  ll->addWidget(smart_curve_.first, 0, Qt::AlignTop | Qt::AlignLeft);

  auto frame = new QFrame();
  frame->setFrameShape(QFrame::Shape::VLine);
  frame->setFrameShadow(QFrame::Shadow::Sunken);
  frame->setContentsMargins(0, 2, 0, 2);
  ll->addWidget(frame, 0, Qt::AlignTop | Qt::AlignLeft);

  transform_menu_ = createOptionButton(QIcon(":/ic_transform"));
  ll->addWidget(transform_menu_, 0, Qt::AlignTop | Qt::AlignLeft);

  proc_menu_ = createOptionButton(QIcon(":/ic_filter"));
  ll->addWidget(proc_menu_, 0, Qt::AlignTop | Qt::AlignLeft);

  ll->addWidget(new QWidget(), 1, Qt::AlignTop | Qt::AlignLeft);

  connect(reset, &QPushButton::clicked, viewport_, &Viewport::fitImageToViewport);
  connect(zoom_menu_, &QPushButton::clicked, this, &MainWindow::showZoomMenu);
  connect(draw_line_.first, &QPushButton::clicked, this, &MainWindow::drawLine);
  connect(draw_angle_.first, &QPushButton::clicked, this, &MainWindow::drawAngle);
  connect(draw_cobb_angle_.first, &QPushButton::clicked, this, &MainWindow::drawCobbAngle);
  connect(draw_circle_.first, &QPushButton::clicked, this, &MainWindow::drawCircle);
  connect(draw_poly_.first, &QPushButton::clicked, this, &MainWindow::drawPoly);
  connect(smart_curve_.first, &QPushButton::clicked, this, &MainWindow::smartCurve);
  connect(proc_menu_, &QPushButton::clicked, this, &MainWindow::showProcMenu);
  connect(transform_menu_, &QPushButton::clicked, this, &MainWindow::showTransformMenu);
  
  proc_menu_->setEnabled(false);
  transform_menu_->setEnabled(false);
}

QPushButton* MainWindow::createOptionButton(QIcon icon, bool enabled) {
  auto button = new QPushButton(icon, "");
  button->setFocusPolicy(Qt::NoFocus);
  button->setFixedSize(24, 24);
  button->setCheckable(true);
  button->setChecked(false);
  button->setEnabled(enabled);

  button->setStyleSheet(
    "QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; }; "
    "QPushButton:!enabled { border-width: 1px; border-style: outset; border-color: black; background-color: gray; }");

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
  if (AppPrefs::contains("window/position")) {
    move(AppPrefs::read("window/position").toPoint());
  }

  if (AppPrefs::contains("window/size")) {
    resize(AppPrefs::read("window/size").toSize());
  }

  initClassifier();
}

void MainWindow::initClassifier() {
  classifier_enabled_ = AppPrefs::read("enable_classifier").toBool();
  classifier_.initFromResource(AppPrefs::read("classifier_params_path").toString());
}

void MainWindow::setItemAsCurrent(Metadata::HardPtr data) {
  // previous item
  if (current_item_ && current_item_ != data) {
    current_item_->viewport_state = viewport_->state();
    current_item_->calib_coef = viewport_->calibCoef();
    current_item_->graphics_items = viewport_->graphicsItems();
  }

  current_item_ = data;
  updateCurrentItem();

  // run classification if needed
  if (classifier_enabled_ && data->joints.isEmpty() && !in_process_.contains(data.get())) {
    if (!in_process_.isEmpty()) {
      process_queue_.push_back(data);
    }
    else {
      in_process_.insert(data.get());
      loading_ind_->startAnimation();

      QtConcurrent::run(this, &MainWindow::runOnData, data);
    }
  }
}

void MainWindow::onItemProcessed(Metadata::HardPtr data) {
  if (!process_queue_.isEmpty()) {
    auto next = process_queue_.takeFirst();
    in_process_.insert(next.get());

    QtConcurrent::run(this, &MainWindow::runOnData, next);
  }
  else if (in_process_.isEmpty()) {
    loading_ind_->stopAnimation();
  }

  if (current_item_ == data) {
    saveCurrentGraphicsItems();
    updateCurrentItem();
  }
}

void MainWindow::calibrateForLength(qreal length) {
  bool ok;
  auto desc = "Enter new distance for current item:";
  auto new_length = QInputDialog::getDouble(this, "Length calibration", desc, 0.0, 0.0, DBL_MAX, 2, &ok);
  if (ok) {
    auto coef = new_length / length;
    viewport_->setCalibrationCoef(coef);
    calib_coef_->setText(QString::number(coef));
    reset_calib_->setEnabled(true);
  }
}

void MainWindow::saveCurrentGraphicsItems() {
  if (current_item_) {
    current_item_->calib_coef = viewport_->calibCoef();
    current_item_->graphics_items = viewport_->graphicsItems();
  }
}

void MainWindow::updateCurrentItem() {
  auto sample = current_item_->image.clone();

  // remove previous graphs
  right_panel_->setRowCount(0);
  right_panel_->setRowCount(current_item_->joints.size());
  for (int k = 0; k < current_item_->joints.size(); ++k) {
    const auto& joint = current_item_->joints[k];

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

  // transformations
  if (const auto r = current_item_->rotation) {
    if (r == 90) cv::rotate(sample, sample, cv::ROTATE_90_CLOCKWISE);
    else if (r == 270) cv::rotate(sample, sample, cv::ROTATE_90_COUNTERCLOCKWISE);
    else if (r == 180) cv::rotate(sample, sample, cv::ROTATE_180);
  }

  for (auto t : current_item_->transformations) {
    if (t == Transformation::HFlip) cv::flip(sample, sample, 1);
    else if (t == Transformation::VFlip) cv::flip(sample, sample, 0);
    else {
      // TODO:
    }
  }

  // currentn item
  viewport_->setImage(sample, current_item_->rotation);
  viewport_->setGradient(current_item_->gradient);
  if (in_process_.isEmpty()) {
    loading_ind_->stopAnimation();
  }

  // set actual image position and scale
  if (!current_item_->already_display) {
    viewport_->fitImageToViewport();
    current_item_->viewport_state = viewport_->state();
    current_item_->already_display = true;
  }
  else {
    viewport_->setState(current_item_->viewport_state);
  }

  // restore graphics items
  viewport_->setCalibrationCoef(current_item_->calib_coef);
  viewport_->setGraphicsItems(current_item_->graphics_items, current_item_->transformations, current_item_->rotation);
  calib_coef_->setText(current_item_->calib_coef ? QString::number(current_item_->calib_coef.value()) : "");

  view_queue_->updateView();
  zoom_menu_->setEnabled(true);
}

void MainWindow::runOnData(Metadata::HardPtr data) {
  auto sample = data->src_image.clone();

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
  emit itemProcessed(data);
}

void MainWindow::rotateCurrentItem(int angle) {
  saveCurrentGraphicsItems();

  current_item_->rotation += angle;

  if (current_item_->rotation < 0) current_item_->rotation += 360;
  else if (current_item_->rotation > 270) current_item_->rotation -= 360;

  updateCurrentItem();
  clearModeView();
}

void MainWindow::applyFilterForCurrent(cv::Mat filter, float delta, bool apply_to_gray) {
  if (current_item_) {
    cv::Mat tmp, output;
    if (!apply_to_gray) tmp = current_item_->src_image;
    else cv::cvtColor(current_item_->src_image, tmp, cv::COLOR_RGB2GRAY);

    cv::filter2D(tmp, output, -1, filter, cv::Point(-1, -1), delta);

    if (!apply_to_gray) current_item_->image = output;
    else cv::cvtColor(output, current_item_->image, cv::COLOR_GRAY2RGB);

    updateCurrentItem();
  }
}

void MainWindow::calibrate(GraphicsItem* item, const QPoint& pt) {
  calibrateForLength(item->length());
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
  auto default_path = AppPrefs::read("last-dicom-path", "").toString();
  auto path = QFileDialog::getOpenFileName(this, "Load DICOM file", default_path, filters);
  if (!path.isEmpty()) {
    gdcm::ImageReader reader;
    reader.SetFileName(path.toStdString().c_str());
    if (reader.Read()) {
      auto filename = QFileInfo(path).fileName();
      auto image = reader.GetImage();

      std::vector<char> buffer(image.GetBufferLength());
      image.GetBuffer(buffer.data());
      
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

      cv::Mat sample;
      cv::Mat img(image.GetRows(), image.GetColumns(), pixel_type, buffer.data());
      cv::cvtColor(img, sample, cv::COLOR_GRAY2RGB);

      AppPrefs::write("last-dicom-path", path.left(path.lastIndexOf('/')) + "/");
      open(filename, sample);
    }
  }
}

void MainWindow::openSample(bool) {
  auto filters = "Image files (*.bmp *.png *.jpg *.jpeg);;";
  auto default_path = AppPrefs::read("last-file-path", "").toString();
  auto path = QFileDialog::getOpenFileName(this, "Load image", default_path, filters);
  if (!path.isEmpty()) {
    auto filename = QFileInfo(path).fileName();
    auto sample = cv::imread(path.toLocal8Bit().data(), cv::IMREAD_COLOR);

    AppPrefs::write("last-file-path", path.left(path.lastIndexOf('/')) + "/");
    open(filename, sample);
  }
}

void MainWindow::openSamples(bool) {
  auto filters = "Image files (*.bmp *.png *.jpg *.jpeg);;";
  auto default_path = AppPrefs::read("last-files-path", "").toString();
  auto files = QFileDialog::getOpenFileNames(this, "Load images...", default_path, filters);
  if (!files.isEmpty()) {
    for (auto path : files) {
      auto filename = QFileInfo(path).fileName();
      auto sample = cv::imread(path.toLocal8Bit().data(), cv::IMREAD_COLOR);
      open(filename, sample);
    }

    AppPrefs::write("last-files-path", files.first().left(files.first().lastIndexOf('/')) + "/");
  }
}

void MainWindow::openFolder(bool) {
  auto default_path = AppPrefs::read("last-dir-path", "").toString();
  auto path = QFileDialog::getOpenFileName(this, "Load directory", default_path, "", nullptr, QFileDialog::ShowDirsOnly);
  if (!path.isEmpty()) {

    AppPrefs::write("last-dir-path", path.left(path.lastIndexOf('/')) + "/");
  }
}

void MainWindow::open(const QString& filename, cv::Mat sample) {
  auto item = std::make_shared<Metadata>();
  item->filename = filename;
  item->src_image = sample.clone();
  item->image = sample;

  view_queue_->addItem(item);

  proc_menu_->setEnabled(true);
  calibrate_->setEnabled(true);
  transform_menu_->setEnabled(true);
}

void MainWindow::setCalibration() {
  if (viewport_->mode() != Viewport::Mode::View) {
    for (auto it : { draw_poly_, draw_circle_, draw_line_, draw_angle_, draw_cobb_angle_ }) {
      it.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
      it.second->setChecked(false);
    }
  }

  viewport_->setMode(Viewport::Mode::Calibrate);
}

void MainWindow::resetCalibration() {
  calib_coef_->setText("");
  viewport_->resetCalibrationCoef();
  reset_calib_->setEnabled(false);
}

void MainWindow::showSettings() {
  if (SettingsWindow::opened) {
    return;
  }

  auto settings_window = new SettingsWindow();
  QObject::connect(settings_window, &SettingsWindow::closed, [=](bool) {
    initClassifier();
  });
  settings_window->show();
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

void MainWindow::drawLine(bool checked) {
  if (viewport_->mode() != Viewport::Mode::DrawLine) {
    viewport_->setMode(Viewport::Mode::DrawLine);
    draw_line_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: rgb(28, 244, 19); } ");
    for (auto it : { draw_poly_, draw_circle_, draw_angle_, draw_cobb_angle_, smart_curve_ }) {
      it.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
      it.second->setChecked(false);
    }
  }
  else {
    viewport_->setMode(Viewport::Mode::View);
    draw_line_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
    draw_line_.second->setChecked(false);
  }
}

void MainWindow::drawCircle(bool checked) {
  if (viewport_->mode() != Viewport::Mode::DrawCircle) {
    viewport_->setMode(Viewport::Mode::DrawCircle);
    draw_circle_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: rgb(28, 244, 19); } ");
    for (auto it : { draw_poly_, draw_line_, draw_angle_, draw_cobb_angle_, smart_curve_ }) {
      it.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
      it.second->setChecked(false);
    }
  }
  else {
    viewport_->setMode(Viewport::Mode::View);
    draw_circle_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
    draw_circle_.second->setChecked(false);
  }
}

void MainWindow::drawAngle(bool checked) {
  if (viewport_->mode() != Viewport::Mode::DrawAngle) {
    viewport_->setMode(Viewport::Mode::DrawAngle);
    draw_angle_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: rgb(28, 244, 19); } ");   
    for (auto it : { draw_poly_, draw_circle_, draw_line_, draw_cobb_angle_, smart_curve_ }) {
      it.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
      it.second->setChecked(false);
    }
  }
  else {
    viewport_->setMode(Viewport::Mode::View);
    draw_angle_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
    draw_angle_.second->setChecked(false);
  }
}

void MainWindow::drawCobbAngle(bool checked) {
  if (viewport_->mode() != Viewport::Mode::DrawCobbAngle) {
    viewport_->setMode(Viewport::Mode::DrawCobbAngle);
    draw_cobb_angle_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: rgb(28, 244, 19); } ");
    for (auto it : { draw_poly_, draw_circle_, draw_line_, draw_angle_, smart_curve_ }) {
      it.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
      it.second->setChecked(false);
    }
  }
  else {
    viewport_->setMode(Viewport::Mode::View);
    draw_cobb_angle_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
    draw_cobb_angle_.second->setChecked(false);
  }
}

void MainWindow::smartCurve(bool) {
  if (viewport_->mode() != Viewport::Mode::SmartCurve) {
    viewport_->setMode(Viewport::Mode::SmartCurve);
    smart_curve_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: rgb(28, 244, 19); } ");
    for (auto it : { draw_circle_, draw_line_, draw_angle_, draw_cobb_angle_, draw_poly_ }) {
      it.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
      it.second->setChecked(false);
    }

    // make gradient for current image
    if (current_item_->gradient.empty()) {
      cv::Mat temp;
      cv::GaussianBlur(current_item_->src_image, temp, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT);
      // Convert the image to grayscale
      cv::cvtColor(temp, temp, cv::COLOR_RGB2GRAY);

      current_item_->gradient = gvf(temp, 0.04, 55);

      // current_item_->gradient.convertTo(current_item_->image, CV_8UC1, 255);
      // cv::cvtColor(current_item_->image, current_item_->image, cv::COLOR_GRAY2RGB);
      // updateCurrentItem();

      viewport_->setGradient(current_item_->gradient);
    }
  }
  else {
    viewport_->setMode(Viewport::Mode::View);
    smart_curve_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
    smart_curve_.second->setChecked(false);
  }
}

void MainWindow::drawPoly(bool checked) {
  if (viewport_->mode() != Viewport::Mode::DrawPoly) {
    viewport_->setMode(Viewport::Mode::DrawPoly);
    draw_poly_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: rgb(28, 244, 19); } ");
    for (auto it : { draw_circle_, draw_line_, draw_angle_, draw_cobb_angle_, smart_curve_ }) {
      it.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
      it.second->setChecked(false);
    }
  }
  else {
    viewport_->setMode(Viewport::Mode::View);
    draw_poly_.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
    draw_poly_.second->setChecked(false);
  }
}

void MainWindow::clearModeView() {
  if (viewport_->mode() != Viewport::Mode::View) {
    viewport_->setMode(Viewport::Mode::View);

    for (auto it : { draw_poly_, draw_circle_, draw_line_, draw_angle_ }) {
      it.first->setStyleSheet("QPushButton { border-width: 1px; border-style: outset; border-color: black; background-color: yellow; } ");
      it.second->setChecked(false);
    }
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

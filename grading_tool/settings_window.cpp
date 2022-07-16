#include "settings_window.h"
#include "settings_window.h"
#include <QApplication>
#include <QJsonDocument>
#include <QGridLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QLabel>
#include <QDebug>

#include "app_preferences.h"

bool SettingsWindow::opened = false;

SettingsWindow::SettingsWindow(QWidget* parent) :
  QMainWindow(parent, Qt::Dialog) 
{
  SettingsWindow::opened = true;

  setWindowTitle("Settings");
  setWindowIcon(QIcon(":/ic_settings"));
  setAttribute(Qt::WA_DeleteOnClose);

  auto w = new QWidget();
  auto vl = new QVBoxLayout(w);
  vl->setAlignment(Qt::AlignTop);

  auto create_selector = [this](QGridLayout* l, QString title, QString name, QString filter, int row) {
    auto caption = new QLabel(title);
    caption->setFixedWidth(115);
    l->addWidget(caption, row, 0, Qt::AlignVCenter);

    auto line_edit = new QLineEdit("");
    line_edit->setObjectName(name);
    l->addWidget(line_edit, row, 1);

    auto button = new QPushButton("...");
    button->setFixedWidth(64);
    l->addWidget(button, row, 2);
    connect(button, &QPushButton::clicked, [this, line_edit, filter, title]() {
      auto default_dir = AppPrefs::read("last_selector_path", line_edit->text()).toString();
      auto filename = QFileDialog::getOpenFileName(this, title, default_dir, filter, nullptr, QFileDialog::DontConfirmOverwrite);
      if (!filename.isEmpty()) {
        AppPrefs::write("last_selector_path", QFileInfo(filename).dir().path());
        line_edit->setText(filename);
      }
    });

    return std::make_tuple(line_edit, button);
  };
  auto create_combo_box = [this](QGridLayout* l, QString title, QString name, int row, QStringList vals) {
    auto caption = new QLabel(title);
    caption->setFixedWidth(150);
    l->addWidget(caption, row, 0);

    auto cb = new QComboBox();
    cb->setObjectName(name);
    cb->addItems(vals);
    l->addWidget(cb, row, 1);

    return cb;
  };
  auto create_check_box = [this](QGridLayout* l, QString title, QString name, int row, int cols = 1, bool fixed_width = false) {
    auto ch = new QCheckBox(title);
    ch->setObjectName(name);
    if (fixed_width) {
      ch->setFixedWidth(250);
    }

    ch->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    l->addWidget(ch, row, 0, 1, cols, Qt::AlignTop | Qt::AlignLeft);
    return ch;
  };
  auto create_item = [this](QGridLayout* l, QString title, QString name, int row) {
    auto caption = new QLabel(title);
    caption->setFixedWidth(250);
    l->addWidget(caption, row, 0);

    auto line_edit = new QLineEdit("");
    line_edit->setObjectName(name);
    l->addWidget(line_edit, row, 1);

    return line_edit;
  };

  //
  auto gb = new QGroupBox("Common settings");
  gb->setStyleSheet("QGroupBox { font-weight: bold; } ");
  gb->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  vl->addWidget(gb);

  int r = 0;
  auto l = new QGridLayout(gb);
  l->setContentsMargins(8, 8, 8, 8);
  l->setAlignment(Qt::AlignTop);
  l->setSpacing(8);

  auto enable_classifier = new QCheckBox("Enable automitic osteoarthritis classification");
  enable_classifier->setObjectName("enable_classifier");
  l->addWidget(enable_classifier, r++, 0, 1, 2);

  create_selector(l, "Classifier file path", "classifier_params_path", "", r++);

  //
  gb = new QGroupBox("Contours extraction");
  gb->setStyleSheet("QGroupBox { font-weight: bold; } ");
  gb->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  vl->addWidget(gb);

  r = 0;
  l = new QGridLayout(gb);
  l->setContentsMargins(8, 8, 8, 8);
  l->setAlignment(Qt::AlignTop);
  l->setSpacing(8);

  auto image_smoothing = create_check_box(l, "Use pre-processing: image smoothing", "image_smoothing", r++);
  auto use_openmp = create_check_box(l, "Use multithreaded contours finder", "use_openmp", r++);
  auto active_contours = create_check_box(l, "Use active contours to improve result", "active_contours", r++);
  auto accurate_split = create_check_box(l, "Use post-processing: accurate split", "accurate_split", r++);

  //
  auto scroll_area = new QScrollArea(this);
  scroll_area->horizontalScrollBar()->hide();
  scroll_area->setBackgroundRole(QPalette::Window);
  scroll_area->setFrameShadow(QFrame::Plain);
  scroll_area->setFrameShape(QFrame::NoFrame);
  scroll_area->setWidgetResizable(true);
  scroll_area->setWidget(w);

  setCentralWidget(scroll_area);
  setFixedSize(480, 320);

  // запись изменений 
  QObject::connect(this, &SettingsWindow::closed, [=](bool) {
    const auto enable_classifier = findChild<QCheckBox*>("enable_classifier");
    const auto classifier_params_path = findChild<QLineEdit*>("classifier_params_path");

    AppPrefs::write("enable_classifier", enable_classifier->isChecked());
    AppPrefs::write("classifier_params_path", classifier_params_path->text());

    AppPrefs::write("image_smoothing", image_smoothing->isChecked());
    AppPrefs::write("use_openmp", use_openmp->isChecked());
    AppPrefs::write("active_contours", active_contours->isChecked());
    AppPrefs::write("accurate_split", accurate_split->isChecked());
  });

  init();
}

SettingsWindow::~SettingsWindow() {
  SettingsWindow::opened = false;
}

void SettingsWindow::hideEvent(QHideEvent* e) {
  emit visibilityChanged(false);
  emit closed(true);

  QMainWindow::hideEvent(e);
}

void SettingsWindow::init() {
  const auto classifier_params_path = findChild<QLineEdit*>("classifier_params_path");
  const auto enable_classifier = findChild<QCheckBox*>("enable_classifier");
  const auto image_smoothing = findChild<QCheckBox*>("image_smoothing");
  const auto use_openmp = findChild<QCheckBox*>("use_openmp");
  const auto active_contours = findChild<QCheckBox*>("active_contours");
  const auto accurate_split = findChild<QCheckBox*>("accurate_split");

  // classifier
  enable_classifier->setChecked(AppPrefs::read("enable_classifier").toBool());
  classifier_params_path->setText(AppPrefs::read("classifier_params_path").toString());
  if (classifier_params_path->text().isEmpty() && QFile("script.zip").exists()) {
    classifier_params_path->setText(QFileInfo("script.zip").absoluteFilePath());
  }

  // contours extractor
  image_smoothing->setChecked(AppPrefs::read("image_smoothing").toBool());
  use_openmp->setChecked(AppPrefs::read("use_openmp").toBool());
  active_contours->setChecked(AppPrefs::read("active_contours").toBool());
  accurate_split->setChecked(AppPrefs::read("accurate_split").toBool());
}

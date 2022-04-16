#pragma once
#include <QMainWindow>

class QGroupBox;
class QListWidget;
class QLabel;

class SettingsWindow : public QMainWindow {
  Q_OBJECT

public:
  static bool opened;

protected:
  QGroupBox* auto_clsf_ = nullptr;
  QListWidget* auto_clsf_plugins_ = nullptr;
  QLabel* fare_warn_ = nullptr;

protected:
  void hideEvent(QHideEvent*) override;

public:
  explicit SettingsWindow(QWidget* parent = nullptr);
  ~SettingsWindow();

  void init();

  Q_SIGNAL void visibilityChanged(bool visible);
  Q_SIGNAL void closed(bool window_closed);
};

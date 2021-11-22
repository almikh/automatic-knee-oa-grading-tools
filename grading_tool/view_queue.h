#pragma once
#include <QTableWidget>

#include "metadata.h"

class QLabel;

class ViewQueue : public QTableWidget {
  Q_OBJECT

private:
  QList<QLabel*> labels_;
  QList<Metadata::HardPtr> data_;

  int getIconHeight() const;

protected:
  void resizeEvent(QResizeEvent* event) override;

  void keyPressEvent(QKeyEvent* e) override;

  void set(int idx);
  void remove(int idx);
  void select(int idx, bool emitSignal = true);

  Q_SLOT void itemSelectionChanged();
  Q_SLOT void itemClicked(int row, int column);

public:
  ViewQueue(QWidget* parent = nullptr);

  int count() const;
  int currentItemIdx() const;

  Q_SLOT void addItem(Metadata::HardPtr item);

  Q_SLOT void updateView();

  Q_SIGNAL void currentItemChanged(Metadata::HardPtr item);
};

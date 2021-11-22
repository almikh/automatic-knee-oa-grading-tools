#include "view_queue.h"
#include <QTableWidgetItem>
#include <QGraphicsView>
#include <QTableWidget>
#include <QHeaderView>
#include <QMouseEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QCheckBox>
#include <QScrollBar>
#include <QLabel>
#include <QDebug>
#include <QMenu>

#include "utils.h"

using namespace convert;

ViewQueue::ViewQueue(QWidget* parent) :
  QTableWidget(1, 0, parent)
{
  setShowGrid(false);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setMinimumHeight(104);

  verticalHeader()->hide();
  horizontalHeader()->hide();

  connect(this, &QTableWidget::itemSelectionChanged, this, &ViewQueue::itemSelectionChanged);
  connect(this, &QTableWidget::cellClicked, this, &ViewQueue::itemClicked);

  qRegisterMetaType<Metadata::HardPtr>("Metadata::HardPtr");
}

void ViewQueue::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);

  if (columnCount() == 0) return;

  const auto s = getIconHeight();
  for (int i = 0; i < columnCount(); ++i) {
    if (auto l = cellWidget(0, i)->findChild<QLabel*>("View")) {
      l->setPixmap(QPixmap::fromImage(cv2qt(data_[i]->image)).scaledToHeight(s));
    }
  }

  resizeColumnsToContents();
  resizeRowsToContents();
}

void ViewQueue::keyPressEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_Delete) {
    auto ranges = selectedRanges();
    if (!ranges.isEmpty() && ranges.front().leftColumn() > 0) {
      remove(ranges.front().leftColumn());
    }
  }
}

void ViewQueue::updateView() {
  const auto s = getIconHeight();
  for (int i = 0; i < columnCount(); ++i) {
    auto l = cellWidget(0, i)->findChild<QLabel*>("View");
    l->setPixmap(QPixmap::fromImage(cv2qt(data_[i]->image)).scaledToHeight(s));
  }

  resizeColumnsToContents();
  resizeRowsToContents();
}

int ViewQueue::getIconHeight() const {
  const auto margins = contentsMargins();
  const int s = height() - 24 - margins.bottom() - margins.top() - horizontalScrollBar()->height();
  return s;
}

void ViewQueue::set(int idx) {
  Q_ASSERT(idx < data_.size());

  auto w = new QWidget(this);
  auto view = new QLabel(w);
  view->setObjectName("View");
  view->setAlignment(Qt::AlignCenter);
  view->setContentsMargins(4, 0, 4, 8);
  view->setPixmap(QPixmap::fromImage(cv2qt(data_[idx]->image)).scaledToHeight(getIconHeight()));

  auto title = new QLabel(data_[idx]->filename, w);
  title->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  title->setContentsMargins(4, 0, 4, 0);
  title->setObjectName("Name");

  auto l = new QVBoxLayout(w);
  l->setMargin(2);
  l->setAlignment(Qt::AlignCenter);
  l->addWidget(title, 0, Qt::AlignHCenter);
  l->addWidget(view);

  setCellWidget(0, idx, w);
}

void ViewQueue::select(int idx, bool emitSignal) {
  if (idx == -1) return;

  Q_ASSERT(idx < data_.size());

  setCurrentCell(0, idx);
  if (emitSignal) {
    emit currentItemChanged(data_[idx]);
  }
}

void ViewQueue::remove(int col) {
  data_.removeAt(col);
  setColumnCount(columnCount() - 1);
  for (int i = col; i < data_.size(); ++i) {
    set(i);
  }

  auto ranges = selectedRanges();
  if (ranges.isEmpty() || ranges.front().leftColumn() < 0) {
    const int idx = columnCount() - 1;
    setCurrentCell(0, idx);
  }
}

int ViewQueue::currentItemIdx() const {
  auto ranges = selectedRanges();
  if (!ranges.isEmpty() && ranges.front().leftColumn() >= 0) {
    return ranges.front().leftColumn();
  }

  return -1;
}

void ViewQueue::addItem(Metadata::HardPtr item) {
  data_ << item;

  int nextCol = columnCount();
  setColumnCount(nextCol + 1);

  set(nextCol);

  resizeColumnsToContents();
  resizeRowsToContents();

  setCurrentCell(0, nextCol);
}

int ViewQueue::count() const {
  return data_.size();
}

void ViewQueue::itemSelectionChanged() {
  const auto col = currentItemIdx();
  if (col >= 0) {
    emit currentItemChanged(data_[col]);
  }
}

void ViewQueue::itemClicked(int row, int column) {
  Q_UNUSED(row);
}

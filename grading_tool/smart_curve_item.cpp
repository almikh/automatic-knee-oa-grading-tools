#include "smart_curve_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

SmartCurveItem::SmartCurveItem(QGraphicsItem* parent) :
  QGraphicsItem(parent)
{
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
  setAcceptedMouseButtons(Qt::LeftButton);
}

bool SmartCurveItem::isHighlighted() const {
  return highlighted_;
}

bool SmartCurveItem::isUnderPos(const QPointF& p) const {
  return false;
}

void SmartCurveItem::setPartUnderMouse(int idx) {
  part_under_mouse_ = idx;
}

void SmartCurveItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;
  update();
}

void SmartCurveItem::setHighlighted(bool selected) {
  highlighted_ = selected;
}

void SmartCurveItem::setPen(const QPen& pen) {
  // TODO:
}

void SmartCurveItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  painter->setPen(QPen(Qt::red, 1.0 / scale_factor_));
  // TODO:
}

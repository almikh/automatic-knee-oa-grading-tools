#include "graphics_cobb_angle_item.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>

GraphicsCobbAngleItem::GraphicsCobbAngleItem(const QPolygonF& poly, QGraphicsItem* parent) :
  QGraphicsLineItem(QLineF(poly[0], poly[0]), parent),
  middle_line_(this),
  second_line_(this),
  polygon_(poly) 
{
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
  setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
  setAcceptedMouseButtons(Qt::LeftButton);
}

void GraphicsCobbAngleItem::setPoint(int idx, const QPointF& pt, bool created) {
  auto poly = polygon_;
  if (poly.size() <= idx) poly.push_back(pt);

  poly[idx] = pt;

  if (created) setPolygon(poly);
  else initLines(poly);
}

bool GraphicsCobbAngleItem::isHighlighted() const {
  return highlighted_;
}

double GraphicsCobbAngleItem::angle() const {
  QPointF fp1 = line().p1().x() < line().p2().x() ? line().p1() : line().p2();
  QPointF fp2 = line().p1().x() > line().p2().x() ? line().p1() : line().p2();
  QPointF sp1 = second_line_.line().p1().x() < second_line_.line().p2().x() ? second_line_.line().p1() : second_line_.line().p2();
  QPointF sp2 = second_line_.line().p1().x() > second_line_.line().p2().x() ? second_line_.line().p1() : second_line_.line().p2();

  auto ans = QLineF(sp1, sp2).angleTo(QLineF(fp1, fp2));
  return ans < 180 ? ans : 360 - ans;
}

QPolygonF GraphicsCobbAngleItem::polygon() const {
  return polygon_;
}

void GraphicsCobbAngleItem::initLines(const QPolygonF& poly) {
  if (poly.count() >= 2) setLine(QLineF(poly[0], poly[1]));
  if (poly.count() > 2) second_line_.setLine(QLineF(poly[2], poly.back()));

  if (poly.count() == 4) {
    auto mp1 = (poly[0] + poly[1]) / 2;
    auto mp2 = (poly[2] + poly[3]) / 2;
    middle_line_.setLine(QLineF(mp1, mp2));
  }
}

void GraphicsCobbAngleItem::setPolygon(const QPolygonF& poly) {
  polygon_ = poly;
  initLines(polygon_);
}

bool GraphicsCobbAngleItem::isUnderPos(const QPointF& p) const {
  return false;
}

void GraphicsCobbAngleItem::setPartUnderMouse(int idx) {
  part_under_mouse_ = idx;
}

void GraphicsCobbAngleItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;
  update();
}

void GraphicsCobbAngleItem::setHighlighted(bool selected) {
  highlighted_ = selected;
}

void GraphicsCobbAngleItem::setPen(const QPen& pen) {
  QGraphicsLineItem::setPen(pen);
  second_line_.setPen(pen);

  auto p2 = pen;
  p2.setWidthF(1.0 / scale_factor_);
  p2.setStyle(Qt::PenStyle::DashLine);
  middle_line_.setPen(p2);
}

void GraphicsCobbAngleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  QGraphicsLineItem::paint(painter, o, w);

  painter->setPen(QPen(Qt::red, 1.0 / scale_factor_));

  auto poly = polygon();
  auto tick_length = qMax(1.0f, 5 / scale_factor_);
  for (int k = 0; k < poly.count(); ++k) {
    const auto pt = poly[k].toPoint();
    painter->setPen(QPen(part_under_mouse_ == k ? Qt::green : Qt::red, 1.0 / scale_factor_));
    painter->drawLine(QLineF(pt - QPointF(tick_length, 0), pt + QPointF(tick_length, 0)));
    painter->drawLine(QLineF(pt - QPointF(0, tick_length), pt + QPointF(0, tick_length)));
  }
}

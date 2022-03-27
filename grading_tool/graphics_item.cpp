#include "graphics_item.h"
#include <QPainter>
#include <QDebug>

GraphicsItem::GraphicsItem(const QLineF& line, QGraphicsItem* parent):
  QGraphicsItem(parent),
  line_(new GraphicsLineItem(line, this)),
  item_(new GraphicsTextItem("", this)),
  type_(Type::Line)
{
  item_->setFont(QFont("Arial", 12 / scale_factor_));
  item_->setBackgroundColor(QColor(214, 169, 56, 88));
  
  mouseMoveEvent(line.p2());
}

GraphicsItem::~GraphicsItem() {
  delete line_;
  delete item_;
}

GraphicsItem* GraphicsItem::makeLine(const QPointF& p1, const QPointF& p2, QGraphicsItem* parent) {
  return new GraphicsItem(QLineF(p1, p2), parent);
}

QRectF GraphicsItem::boundingRect() const {
  return line_->boundingRect();
}

bool GraphicsItem::isSelected() const {
  return highlighted_;
}

bool GraphicsItem::isUnderPos(const QPointF& p) const {
  return line_->isUnderPos(p) || item_->isUnderMouse();
}

bool GraphicsItem::isValid() const {
  return line_->line().length() >= 3;
}

bool GraphicsItem::isPartUnderPos(const QPointF& coord) const {
  auto data = line_->line();
  if (std::sqrt(std::pow(coord.x() - data.p1().x(), 2) + std::pow(coord.y() - data.p1().y(), 2)) < 7) {
    line_->setLine(QLineF(data.p2(), data.p1()));
    return true;
  }
  else if (std::sqrt(std::pow(coord.x() - data.p2().x(), 2) + std::pow(coord.y() - data.p2().y(), 2)) < 7) {
    return true;
  }

  return false;
}

void GraphicsItem::setPen(const QColor& color) {
  line_->setPen(QPen(color, 2 / scale_factor_));
}

void GraphicsItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;

  line_->setPen(QPen(Qt::red, 2 / scale_factor_));
  item_->setFont(QFont("Arial", 12 / scale_factor_));
  item_->setPos((line_->line().p1().x() > line_->line().p2().x() ? line_->line().p1() : line_->line().p2()) + QPointF(7, -10 / scale_factor_));

  update();
}

void GraphicsItem::setHighlighted(bool selected) {
  if (selected) {
    item_->setBackgroundColor(QColor(250, 250, 0, 88));
    setPen(Qt::yellow);
  }
  else {
    item_->setBackgroundColor(QColor(214, 169, 56, 88));
    setPen(Qt::red);
  }

  update();
}

void GraphicsItem::setSelected(bool selected) {
  highlighted_ = selected;
  setHighlighted(selected);
}

void GraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  if (type_ == Type::Line) {
    painter->setPen(QPen(Qt::black, 1.0));
    painter->setBrush(QBrush(line_->pen().color()));
    painter->drawEllipse(line_->line().p1(), 4 / scale_factor_, 4 / scale_factor_);
    painter->drawEllipse(line_->line().p2(), 4 / scale_factor_, 4 / scale_factor_);
  }
}

bool GraphicsItem::checkSelection(const QPointF& pos) {
  setHighlighted(isUnderPos(pos));
  return highlighted_;
}

void GraphicsItem::mousePressEvent(const QPointF& pos) {

}

void GraphicsItem::mouseReleaseEvent(const QPointF& pos) {

}

void GraphicsItem::mouseMoveEvent(const QPointF& pos) {
  auto line = line_->line();
  line.setP2(QPointF(pos));
  line_->setLine(line);

  item_->setPlainText(QString::number(line.length(), 'f', 2) + " px");
  item_->setPos((line.p1().x() > line.p2().x() ? line.p1() : line.p2()) + QPointF(7, -10 / scale_factor_));
}

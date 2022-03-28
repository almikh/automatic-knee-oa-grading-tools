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

  mouseMoveEvent(line.p2());
  updateColors();
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

double GraphicsItem::length() const {
  if (type_ == Type::Line) {
    return line_->line().length();
  }

  return 0.0;
}

bool GraphicsItem::isSelected() const {
  return selected_;
}

bool GraphicsItem::isItemUnderMouse() const {
  return item_->isUnderMouse();
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

void GraphicsItem::setCalibrationCoef(std::optional<qreal> coef) {
  calib_coef_ = coef;
  updateCaption();
}

void GraphicsItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;

  item_->setFont(QFont("Arial", 12 / scale_factor_));
  item_->setPos((line_->line().p1().x() > line_->line().p2().x() ? line_->line().p1() : line_->line().p2()) + QPointF(7, -10 / scale_factor_));

  updateColors();
  update();
}

void GraphicsItem::setSelected(bool selected) {
  selected_ = selected;
  updateColors();
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
  highlighted_ = isUnderPos(pos);
  
  updateColors();

  return highlighted_;
}

void GraphicsItem::updateColors() {
  if (highlighted_ || selected_) {
    item_->setBackgroundColor(QColor(250, 250, 0, 88));
    line_->setPen(QPen(Qt::yellow, 2 / scale_factor_));
  }
  else if (calib_coef_) {
    item_->setBackgroundColor(QColor(0, 255, 0, 88));
    line_->setPen(QPen(Qt::green, 2 / scale_factor_));
  }
  else {
    item_->setBackgroundColor(QColor(255, 0, 0, 88));
    line_->setPen(QPen(Qt::red, 2 / scale_factor_));
  }

  update();
}

void GraphicsItem::updateCaption() {
  auto line = line_->line();
  if (calib_coef_) {
    item_->setPlainText(QString::number(line.length() * calib_coef_.value(), 'f', 2) + " mm");
    item_->setPos((line.p1().x() > line.p2().x() ? line.p1() : line.p2()) + QPointF(7, -10 / scale_factor_));
  }
  else {
    item_->setPlainText(QString::number(line.length(), 'f', 2) + " px");
    item_->setPos((line.p1().x() > line.p2().x() ? line.p1() : line.p2()) + QPointF(7, -10 / scale_factor_));
  }

  update();
}

void GraphicsItem::mousePressEvent(const QPointF& pos) {

}

void GraphicsItem::mouseReleaseEvent(const QPointF& pos) {

}

void GraphicsItem::mouseMoveEvent(const QPointF& pos) {
  auto line = line_->line();
  line.setP2(QPointF(pos));
  line_->setLine(line);

  updateCaption();
}

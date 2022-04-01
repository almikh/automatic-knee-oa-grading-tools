#include "graphics_item.h"
#include <QPainter>
#include <QDebug>
#include <QtMath>

qreal dist(const QPointF& p1, const QPointF& p2) {
  return std::sqrt((p1.x() - p2.x())* (p1.x() - p2.x()) + (p1.y() - p2.y())*(p1.y() - p2.y()));
}

GraphicsItem::GraphicsItem(const QLineF& line, QGraphicsItem* parent):
  QGraphicsItem(parent),
  item_(new GraphicsTextItem("", this))
{
  item_->setFont(QFont("Arial", 10 / scale_factor_));
}

GraphicsItem::~GraphicsItem() {
  delete line_;
  delete item_;
}

GraphicsItem* GraphicsItem::makeLine(const QPointF& p1, const QPointF& p2, QGraphicsItem* parent) {
  auto item = new GraphicsItem(QLineF(p1, p2), parent);
  item->line_ = new GraphicsLineItem(QLineF(p1, p2), item);
  item->type_ = Type::Line;

  item->mouseMoveEvent(p2, cv::Mat());
  item->updateColors();

  return item;
}

GraphicsItem* GraphicsItem::makeEllipse(const QPointF& p1, const QPointF& p2, QGraphicsItem* parent) {
  auto item = new GraphicsItem(QLineF(p1, p2), parent);
  item->ellipse_ = new GraphicsEllipseItem(QRect(p1.toPoint(), p2.toPoint()), item);
  item->type_ = Type::Ellipse;

  item->mouseMoveEvent(p2, cv::Mat());
  item->updateColors();

  return item;
}

QRectF GraphicsItem::boundingRect() const {
  if (type_ == Type::Line) {
    return line_->boundingRect();
  }
  else if(type_ == Type::Ellipse) {
    return ellipse_->boundingRect();
  }

  return QRectF();
}

GraphicsItem::Type GraphicsItem::getType() const {
  return type_;
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
  if (type_ == Type::Line) {
    return line_->isUnderPos(p) || item_->isUnderMouse();
  }
  else if (type_ == Type::Ellipse) {
    return  item_->isUnderMouse(); // || ellipse_->isUnderPos(p);
  }

  return false;
}

bool GraphicsItem::isValid() const {
  if (type_ == Type::Line) {
    return line_->line().length() >= 3;
  }
  else if (type_ == Type::Ellipse) {
    auto sq = qAbs(ellipse_->rect().width() * ellipse_->rect().height());
    return sq > 40;
  }

  return false;
}

bool GraphicsItem::isPartUnderPos(const QPointF& coord) const {
  if (type_ == Type::Line) {
    auto data = line_->line();
    if (dist(coord, data.p1()) < 7) {
      line_->setLine(QLineF(data.p2(), data.p1()));
      return true;
    }
    else if (dist(coord, data.p2()) < 7) {
      return true;
    }
  }
  else if (type_ == Type::Ellipse) {
    auto rect = ellipse_->rect();
    for (auto pt : { rect.topLeft(), rect.topRight(), rect.bottomLeft(), rect.bottomRight() }) {
      if (dist(coord, pt) < 7) {
        return true;
      }
    }
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

  if (line_) line_->setScaleFactor(scale_factor_);
  if (ellipse_) ellipse_->setScaleFactor(scale_factor_);

  updateCaption();
  updateColors();
  update();
}

void GraphicsItem::setSelected(bool selected) {
  selected_ = selected;
  updateColors();
}

bool GraphicsItem::checkSelection(const QPointF& pos) {
  highlighted_ = isUnderPos(pos);
  
  updateColors();

  return highlighted_;
}

void GraphicsItem::updateColors() {
  if (highlighted_ || selected_) {
    item_->setBackgroundColor(QColor(80, 80, 0, 200));
    if (line_) line_->setPen(QPen(Qt::yellow, 2 / scale_factor_));
    if (ellipse_) ellipse_->setPen(QPen(Qt::yellow, 2 / scale_factor_));
  }
  else if (calib_coef_) {
    item_->setBackgroundColor(QColor(0, 100, 0, 200));
    if (line_) line_->setPen(QPen(QColor(0, 200, 0), 2 / scale_factor_));
    if (ellipse_) ellipse_->setPen(QPen(QColor(0, 200, 0), 2 / scale_factor_));
  }
  else {
    item_->setBackgroundColor(QColor(80, 80, 0, 200));
    if (line_) line_->setPen(QPen(Qt::red, 2 / scale_factor_));
    if (ellipse_) ellipse_->setPen(QPen(Qt::red, 2 / scale_factor_));
  }

  update();
}

void GraphicsItem::updateCaption() {
  if (type_ == Type::Line) {
    auto line = line_->line();
    if (calib_coef_) {
      item_->setPlainText(QString::number(line.length() * calib_coef_.value(), 'f', 2) + " mm");
    }
    else {
      item_->setPlainText(QString::number(line.length(), 'f', 2) + " px");
    }

    item_->setPos((line.p1().x() > line.p2().x() ? line.p1() : line.p2()) + QPointF(7, -10 / scale_factor_));
  }
  else if (type_ == Type::Ellipse) {
    auto rect = ellipse_->rect();
    if (calib_coef_) {
      item_->setPlainText(
        "Sq: " + QString::number(M_PI * qAbs(rect.width()/2 * rect.height()/2) * calib_coef_.value(), 'f', 2) + " mm\n"
        "Min: " + QString::number(min_ ? min_.value() : 0.0, 'f', 2) + " Max: " + QString::number(max_ ? max_.value() : 0.0, 'f', 2) + "\n"
        "Avg: " + QString::number(avg_ ? avg_.value() : 0.0, 'f', 2));
    }
    else {
      item_->setPlainText(
        "Area: " + QString::number(M_PI * qAbs(rect.width() / 2 * rect.height() / 2), 'f', 2) + " px\n"
        "Min: " + QString::number(min_ ? min_.value() : 0.0, 'f', 2) + " Max: " + QString::number(max_ ? max_.value() : 0.0, 'f', 2) + "\n"
        "Avg: " + QString::number(avg_ ? avg_.value() : 0.0, 'f', 2));
    }

    auto y = (rect.bottomRight().y() + rect.topRight().y()) / 2;
    item_->setPos(QPointF(qMax(rect.left(), rect.right()), y) + QPointF(7, -10 / scale_factor_));
  }

  update();
}

void GraphicsItem::mousePressEvent(const QPointF& pos) {
  if (type_ == Type::Ellipse) {
    auto rect = ellipse_->rect();
    if (dist(rect.bottomLeft(), pos) < 7) {
      h_anchor_ = false;
      v_anchor_ = false;
    }
    else if (dist(rect.bottomRight(), pos) < 7) {
      h_anchor_ = true;
      v_anchor_ = false;
    }
    else if (dist(rect.topLeft(), pos) < 7) {
      h_anchor_ = false;
      v_anchor_ = true;
    }
    else if (dist(rect.topRight(), pos) < 7) {
      h_anchor_ = true;
      v_anchor_ = true;
    }
    else {
      h_anchor_ = false;
      v_anchor_ = false;
    }
  }
}

void GraphicsItem::mouseReleaseEvent(const QPointF& pos) {

}

void GraphicsItem::mouseMoveEvent(const QPointF& pos, const cv::Mat& image) {
  if (type_ == Type::Line) {
    auto line = line_->line();
    line.setP2(QPointF(pos));
    line_->setLine(line);
  }
  else if (type_ == Type::Ellipse) { 
    auto rect = ellipse_->rect();
    if (!h_anchor_ && !v_anchor_) {
      rect.setBottomLeft(pos);
    }
    else if (h_anchor_ && !v_anchor_) {
      rect.setBottomRight(pos);
    }
    else if (!h_anchor_ && v_anchor_) {
      rect.setTopLeft(pos);
    }
    else if (h_anchor_ && v_anchor_) {
      rect.setTopRight(pos);
    }

    int sum = 0, count = 0, max = 0, min = INT_MAX;
    int left = qMax<int>(0, qMin(rect.x(), rect.x() + rect.width()));
    int bottom = qMax<int>(0, qMin(rect.y(), rect.y() + rect.height()));
    int right = qMin(left + qAbs<int>(rect.width()), image.cols - 1), top = qMin<int>(bottom + qAbs(rect.height()), image.rows - 1);
    for (int i = left; i <= right; ++i) {
      for (int j = bottom; j <= top; ++j) {
        auto px = image.at<cv::Vec2b>(j, i);
        min = qMin<int>(px[0], min);
        max = qMax<int>(px[0], max);
        sum += px[0];
        count += 1;
      }
    }

    min_ = min;
    max_ = max;
    avg_ = double(sum) / count;
    updateCaption();

    ellipse_->setRect(rect);
  }

  updateCaption();
}

void GraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {

}

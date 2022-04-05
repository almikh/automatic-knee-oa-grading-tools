#include "graphics_item.h"
#include <QPainter>
#include <QDebug>
#include <QtMath>

#include "utils.h"

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
  item->anchor_index_ = -1;

  item->mouseMoveEvent(p2, cv::Mat());
  item->updateColors();

  return item;
}

GraphicsItem* GraphicsItem::makeAngle(const QPointF& pt, QGraphicsItem* parent) {
  auto item = new GraphicsItem(QLineF(pt, pt), parent);
  item->angle_ = new GraphicsAngleItem(QPolygonF({ pt, pt }), item);
  item->type_ = Type::Angle;
  item->anchor_index_ = -1;

  item->mouseMoveEvent(pt, cv::Mat());
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

QPolygonF GraphicsItem::polygon() const {
  if (angle_) return angle_->polygon();
  else return QPolygonF();
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
  else if (type_ == Type::Ellipse || type_ == Type::Angle) {
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
  else if (type_ == Type::Angle) {
    auto poly = angle_->polygon();
    return poly.count() > 2 && dist(poly[0], poly[1]) > 7 && dist(poly[1], poly[2]) > 7;
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
  else if (type_ == Type::Angle) {
    auto poly = angle_->polygon();
    for (int k = 0; k < poly.count(); ++k) {
      if (dist(coord, poly[k].toPoint()) < 7) {
        return true;
      }
    }
  }

  return false;
}

void GraphicsItem::setPolygon(const QPolygonF& poly) {
  if (angle_) {
    if (angle_->polygon().size() <= 2 && poly.size() > 2) {
      anchor_index_ = 2;
    }

    angle_->setPolygon(poly);
  }
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
  if (angle_) angle_->setScaleFactor(scale_factor_);

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
    if (angle_) angle_->setPen(QPen(Qt::yellow, 2 / scale_factor_));
  }
  else if (calib_coef_) {
    item_->setBackgroundColor(QColor(0, 88, 0, 200));
    if (line_) line_->setPen(QPen(QColor(0, 150, 0), 2 / scale_factor_));
    if (ellipse_) ellipse_->setPen(QPen(QColor(0, 150, 0), 2 / scale_factor_));
    if (angle_) angle_->setPen(QPen(QColor(0, 150, 0), 2 / scale_factor_));
  }
  else {
    item_->setBackgroundColor(QColor(80, 80, 0, 200));
    if (line_) line_->setPen(QPen(Qt::red, 2 / scale_factor_));
    if (ellipse_) ellipse_->setPen(QPen(Qt::red, 2 / scale_factor_));
    if (angle_) angle_->setPen(QPen(Qt::red, 2 / scale_factor_));
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
    auto y = (rect.bottomRight().y() + rect.topRight().y()) / 2;
    if (calib_coef_) {
      item_->setPlainText(
        "Area: " + QString::number(area_.value_or(0) * calib_coef_.value()) + " mm\n"
        "Min: " + QString::number(min_.value_or(0)) + "  Max: " + QString::number(max_.value_or(0)) + "\n"
        "Avg: " + QString::number(avg_.value_or(0)));
    }
    else {
      item_->setPlainText(
        "Area: " + QString::number(area_.value_or(0)) + " px\n"
        "Min: " + QString::number(min_.value_or(0)) + "  Max: " + QString::number(max_.value_or(0)) + "\n"
        "Avg: " + QString::number(avg_.value_or(0)));
    }

    item_->setPos(QPointF(qMax(rect.left(), rect.right()), y) + QPointF(11, -10 / scale_factor_));
  }
  else if (type_ == Type::Angle) {
    auto rect = angle_->boundingRect();
    auto y = (rect.bottomRight().y() + rect.topRight().y()) / 2;

    item_->setPlainText("Angle: " + QString::number(angle_->angle()));
    item_->setPos(QPointF(qMax(rect.left(), rect.right()), y) + QPointF(11, -10 / scale_factor_));
  }

  update();
}

void GraphicsItem::mousePressEvent(const QPointF& pos) {
  anchor_index_ = -1;

  if (type_ == Type::Ellipse) {
    auto rect = ellipse_->rect();
    if (dist(rect.bottomLeft(), pos) < 7) anchor_index_ = 0;
    else if (dist(rect.topLeft(), pos) < 7) anchor_index_ = 1;
    else if (dist(rect.topRight(), pos) < 7) anchor_index_ = 2;
    else if (dist(rect.bottomRight(), pos) < 7) anchor_index_ = 3;
  }
  else if (type_ == Type::Angle) {
    auto poly = angle_->polygon();   

    // when angle item is in creating
    for (int k = poly.count() - 1; k > 0; --k) {
      if (dist(poly[k], poly[k - 1]) < 1) {
        anchor_index_ = k;
        return;
      }
    }

    // when angle item created
    for (int k = 0; k < poly.count(); ++k) {
      if (dist(poly[k], pos) < 7) {
        anchor_index_ = k;
        break;
      }
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
    if (anchor_index_ >= 0) {
      ellipse_->setPoint(anchor_index_, pos);
    }
    else {
      ellipse_->setPoint(3, pos);
    }

    auto rect = ellipse_->rect();
    int sum = 0, count = 0, max = 0, min = INT_MAX;
    int a = rect.width() / 2, b = rect.height() / 2;
    float h = rect.x() + a, k = rect.y() + b, a2 = a*a, b2 = b*b;
    int left = qMax<int>(0, qMin(rect.x(), rect.x() + rect.width()));
    int bottom = qMax<int>(0, qMin(rect.y(), rect.y() + rect.height()));
    int right = qMin(left + qAbs<int>(rect.width()), image.cols - 1), top = qMin<int>(bottom + qAbs(rect.height()), image.rows - 1);
    for (int i = left; i <= right; ++i) {
      for (int j = bottom; j <= top; ++j) {
        if ((i - h) * (i - h) / a2 + (j - k) * (j - k) / b2 <= 1) {
          auto px = image.at<cv::Vec2b>(j, i);
          min = qMin<int>(px[0], min);
          max = qMax<int>(px[0], max);
          sum += px[0];
          count += 1;
        }
      }
    }

    min_ = min;
    max_ = max;
    area_ = count;
    avg_ = double(sum) / count;
    ellipse_->setRect(rect);
  }
  else if (type_ == Type::Angle) {
    if (anchor_index_ >= 0) {
      angle_->setPoint(anchor_index_, pos);
    }
    else {
      angle_->setPoint(angle_->polygon().count() - 1, pos);
    }
  }

  updateCaption();
}

void GraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {

}

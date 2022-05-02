#include "graphics_item.h"
#include <QPainter>
#include <QDebug>
#include <QtMath>

#include "utils.h"

float GraphicsItem::base_touch_radius = 7.0f;

GraphicsItem::GraphicsItem(QGraphicsItem* parent):
  QGraphicsItem(parent),
  item_(new GraphicsTextItem("", this)),
  selected_color_(Qt::yellow),
  default_color_(Qt::red),
  calibrated_color_(0, 150, 0)
{
  item_->setFont(QFont("Arial", 10 / scale_factor_));
}

GraphicsItem::~GraphicsItem() {
  delete line_;
  delete item_;
}

GraphicsItem* GraphicsItem::makeLine(const QPointF& p1, const QPointF& p2, QGraphicsPixmapItem* parent) {
  auto item = new GraphicsItem(parent);
  item->line_ = new GraphicsLineItem(QLineF(p1, p2), item);
  item->type_ = Type::Line;

  item->mouseMoveEvent(p2, cv::Mat());
  item->updateColors();

  return item;
}

GraphicsItem* GraphicsItem::makeEllipse(const QPointF& p1, const QPointF& p2, QGraphicsPixmapItem* parent) {
  auto item = new GraphicsItem(parent);
  item->ellipse_ = new GraphicsEllipseItem(QRect(p1.toPoint(), p2.toPoint()), item);
  item->type_ = Type::Ellipse;
  item->anchor_index_ = -1;

  item->mouseMoveEvent(p2, cv::Mat());
  item->updateColors();

  return item;
}

GraphicsItem* GraphicsItem::makeAngle(const QPointF& pt, QGraphicsPixmapItem* parent) {
  auto item = new GraphicsItem(parent);
  item->angle_ = new GraphicsAngleItem(QPolygonF({ pt, pt }), item);
  item->type_ = Type::Angle;
  item->anchor_index_ = -1;

  item->mouseMoveEvent(pt, cv::Mat());
  item->updateColors();

  return item;
}

GraphicsItem* GraphicsItem::makeCobbAngle(const QPointF& pt, QGraphicsPixmapItem* parent) {
  auto item = new GraphicsItem(parent);
  item->cobb_angle_ = new GraphicsCobbAngleItem(QPolygonF({ pt }), item);
  item->type_ = Type::CobbAngle;
  item->anchor_index_ = -1;

  item->mouseMoveEvent(pt, cv::Mat());
  item->updateColors();

  return item;
}

GraphicsItem* GraphicsItem::makePoly(const QPointF& pt, QGraphicsPixmapItem* parent) {
  auto item = new GraphicsItem(parent);
  item->poly_ = new GraphicsPolyItem(QPolygonF({ pt, pt }), item);
  item->type_ = Type::Poly;
  item->anchor_index_ = -1;

  item->mouseMoveEvent(pt, cv::Mat());
  item->updateColors();

  return item;
}

GraphicsItem* GraphicsItem::makeSmartCurve(const QPointF& pt, QGraphicsPixmapItem* parent) {
  auto item = new GraphicsItem(parent);
  item->smart_curve_ = new SmartCurveItem({ pt.toPoint() }, item);
  item->type_ = Type::SmartCurve;
  item->anchor_index_ = -1;

  item->mouseMoveEvent(pt, cv::Mat());
  item->updateColors();

  return item;
}

GraphicsItem* GraphicsItem::makeFromJson(const QJsonObject& json, const QVector<Transformation>& transforms, int r, QGraphicsPixmapItem* parent) {
  Q_UNUSED(r);

  auto item = new GraphicsItem(parent);
  item->rotation = r;

  auto type = json["type"].toString();
  auto sz = parent->pixmap().size();
  if (type == "line") {
    auto p1 = rotatedPoint(str2point(json["p1"].toString()), r, sz);
    auto p2 = rotatedPoint(str2point(json["p2"].toString()), r, sz);

    for (auto t : transforms) {
      if (t == Transformation::HFlip) {
        item->h_flipped = !item->h_flipped;
        p1.setX(sz.width() - p1.x());
        p2.setX(sz.width() - p2.x());
      }
      else if (t == Transformation::VFlip) {
        item->v_flipped = !item->v_flipped;
        p1.setY(sz.height() - p1.y());
        p2.setY(sz.height() - p2.y());
      }
    }


    item->line_ = new GraphicsLineItem(QLineF(p1, p2), item);
    item->type_ = Type::Line;

    item->mouseMoveEvent(p2, cv::Mat());
    item->updateColors();
  }
  else if (type == "angle") {
    auto p1 = rotatedPoint(str2point(json["p1"].toString()), r, sz);
    auto p2 = rotatedPoint(str2point(json["p2"].toString()), r, sz);
    auto p3 = rotatedPoint(str2point(json["p3"].toString()), r, sz);

    for (auto t : transforms) {
      if (t == Transformation::HFlip) {
        item->h_flipped = !item->h_flipped;
        p1.setX(sz.width() - p1.x());
        p2.setX(sz.width() - p2.x());
        p3.setX(sz.width() - p3.x());
      }
      else if (t == Transformation::VFlip) {
        item->v_flipped = !item->v_flipped;
        p1.setY(sz.height() - p1.y());
        p2.setY(sz.height() - p2.y());
        p3.setY(sz.height() - p3.y());
      }
    }

    item->angle_ = new GraphicsAngleItem(QPolygonF({ p1, p2 }), item);
    item->type_ = Type::Angle;
    item->anchor_index_ = -1;

    item->angle_->setPolygon(QPolygonF({ p1, p2, p3 }));
    item->mouseMoveEvent(p3, cv::Mat());
    item->updateColors();
  }
  else if (type == "cobb_angle") {
    QPolygonF poly;
    for (int k = 0; k < 4; ++k) {
      auto name = "p" + QString::number(k + 1);
      if (json.contains(name)) {
        auto pt = rotatedPoint(str2point(json[name].toString()), r, sz);
        for (auto t : transforms) {
          if (t == Transformation::HFlip) {
            item->h_flipped = !item->h_flipped;
            pt.setX(sz.width() - pt.x());
          }
          else if (t == Transformation::VFlip) {
            item->v_flipped = !item->v_flipped;
            pt.setY(sz.height() - pt.y());
          }
        }

        poly.push_back(pt);
      }
    }

    item->cobb_angle_ = new GraphicsCobbAngleItem(QPolygonF({ poly[0] }), item);
    item->type_ = Type::CobbAngle;
    item->anchor_index_ = -1;

    item->cobb_angle_->setPolygon(poly);
    item->mouseMoveEvent(poly.back(), cv::Mat());
    item->updateColors();
  }
  else if (type == "ellipse") {
    auto tl = rotatedPoint(str2point(json["tl"].toString()), r, sz);
    auto br = rotatedPoint(str2point(json["br"].toString()), r, sz);

    for (auto t : transforms) {
      if (t == Transformation::HFlip) {
        item->h_flipped = !item->h_flipped;
        tl.setX(sz.width() - tl.x());
        br.setX(sz.width() - br.x());
      }
      else if (t == Transformation::VFlip) {
        item->v_flipped = !item->v_flipped;
        tl.setY(sz.height() - tl.y());
        br.setY(sz.height() - br.y());
      }
    }

    item->ellipse_ = new GraphicsEllipseItem(QRect(tl.toPoint(), br.toPoint()), item);
    item->type_ = Type::Ellipse;
    item->anchor_index_ = -1;

    item->mouseMoveEvent(br, cv::Mat());
    item->updateColors();
  }
  else if (type == "poly") {
    QPolygonF poly;
    for (int k = 0; k < json["count"].toInt(); ++k) {
      auto pt = rotatedPoint(str2point(json["p" + QString::number(k)].toString()), r, sz);
      poly.push_back(pt);
    }

    for (auto t : transforms) {
      if (t == Transformation::HFlip) {
        item->h_flipped = !item->h_flipped;
        for (int k = 0; k < poly.size(); ++k) {
          poly[k].setX(sz.width() - poly[k].x());
        }
      }
      else if (t == Transformation::VFlip) {
        item->v_flipped = !item->v_flipped;
        for (int k = 0; k < poly.size(); ++k) {
          poly[k].setY(sz.height() - poly[k].y());
        }
      }
    }

    item->poly_ = new GraphicsPolyItem(poly, item);
    item->type_ = Type::Poly;
    item->anchor_index_ = -1;

    item->mouseMoveEvent(poly.back(), cv::Mat());
    item->updateColors();
  }
  else if (type == "smart_curve") {
    QVector<QPoint> points;
    for (int k = 0; k < json["count"].toInt(); ++k) {
      auto pt = rotatedPoint(str2point(json["p" + QString::number(k)].toString()), r, sz).toPoint();
      points.push_back(pt);
    }

    for (auto t : transforms) {
      if (t == Transformation::HFlip) {
        item->h_flipped = !item->h_flipped;
        for (int k = 0; k < points.size(); ++k) {
          points[k].setX(sz.width() - points[k].x());
        }
      }
      else if (t == Transformation::VFlip) {
        item->v_flipped = !item->v_flipped;
        for (int k = 0; k < points.size(); ++k) {
          points[k].setY(sz.height() - points[k].y());
        }
      }
    }

    item->smart_curve_ = new SmartCurveItem(points, item);
    item->type_ = Type::SmartCurve;
    item->anchor_index_ = -1;

    item->mouseMoveEvent(points.back(), cv::Mat());
    item->updateColors();
  }

  return item;
}

QRectF GraphicsItem::boundingRect() const {
  if (type_ == Type::Line) {
    return line_->boundingRect();
  }
  else if(type_ == Type::Ellipse) {
    return ellipse_->boundingRect();
  }
  else if (type_ == Type::Poly) {
    return poly_->boundingRect();
  }
  else if (type_ == Type::SmartCurve) {
    return smart_curve_->boundingRect();
  }

  return QRectF();
}

QJsonObject GraphicsItem::toJson() const {
  auto parent = dynamic_cast<QGraphicsPixmapItem*>(parentItem());
  auto sz = parent->pixmap().size();

  QJsonObject json;
  if (type_ == Type::Line) {
    auto p1 = line_->line().p1();
    auto p2 = line_->line().p2();

    if (h_flipped) {
      p1.setX(sz.width() - p1.x());
      p2.setX(sz.width() - p2.x());
    }

    if (v_flipped) {
      p1.setY(sz.height() - p1.y());
      p2.setY(sz.height() - p2.y());
    }

    json["type"] = "line";
    json["p1"] = point2str(rotatedPoint(p1, rotation, sz, false));
    json["p2"] = point2str(rotatedPoint(p2, rotation, sz, false));
  }
  else if (type_ == Type::Angle) {
    auto poly = angle_->polygon();

    if (h_flipped) {
      poly[0].setX(sz.width() - poly[0].x());
      poly[1].setX(sz.width() - poly[1].x());
      poly[2].setX(sz.width() - poly[2].x());
    }

    if (v_flipped) {
      poly[0].setY(sz.height() - poly[0].y());
      poly[1].setY(sz.height() - poly[1].y());
      poly[2].setY(sz.height() - poly[2].y());
    }

    json["type"] = "angle";
    json["p1"] = point2str(rotatedPoint(poly[0], rotation, sz, false));
    json["p2"] = point2str(rotatedPoint(poly[1], rotation, sz, false));
    json["p3"] = point2str(rotatedPoint(poly[2], rotation, sz, false));
  }
  else if (type_ == Type::CobbAngle) {
    auto poly = cobb_angle_->polygon();

    json["type"] = "cobb_angle";
    for (int k = 0; k < poly.size(); ++k) {
      if (h_flipped) poly[k].setX(sz.width() - poly[k].x());
      if (v_flipped) poly[k].setY(sz.height() - poly[k].y());

      json["p" + QString::number(k + 1)] = point2str(rotatedPoint(poly[k], rotation, sz, false));
    }
  }
  else if (type_ == Type::Ellipse) {
    auto rect = ellipse_->rect();

    auto tl = rect.topLeft();
    auto br = rect.bottomRight();

    if (h_flipped) {
      tl.setX(sz.width() - tl.x());
      br.setX(sz.width() - br.x());
    }

    if (v_flipped) {
      tl.setY(sz.height() - tl.y());
      br.setY(sz.height() - br.y());
    }

    json["type"] = "ellipse";
    json["tl"] = point2str(rotatedPoint(tl, rotation, sz, false));
    json["br"] = point2str(rotatedPoint(br, rotation, sz, false));
  }
  else if (type_ == Type::Poly) {
    auto poly = poly_->polygon();

    if (h_flipped) {
      for (int k = 0; k < poly.size(); ++k) {
        poly[k].setX(sz.width() - poly[k].x());
      }
    }

    if (v_flipped) {
      for (int k = 0; k < poly.size(); ++k) {
        poly[k].setY(sz.height() - poly[k].y());
      }
    }

    json["type"] = "poly";
    json["count"] = poly.size();
    for (int k = 0; k < poly.size(); ++k) {
      json["p" + QString::number(k)] = point2str(rotatedPoint(poly[k], rotation, sz, false));
    }
  }
  else if (type_ == Type::SmartCurve) {
    auto points = smart_curve_->points();

    if (h_flipped) {
      for (int k = 0; k < points.size(); ++k) {
        points[k].setX(sz.width() - points[k].x());
      }
    }

    if (v_flipped) {
      for (int k = 0; k < points.size(); ++k) {
        points[k].setY(sz.height() - points[k].y());
      }
    }

    json["type"] = "poly";
    json["count"] = points.size();
    for (int k = 0; k < points.size(); ++k) {
      json["p" + QString::number(k)] = point2str(rotatedPoint(points[k], rotation, sz, false));
    }
  }

  return json;
}

QPolygonF GraphicsItem::polygon() const {
  if (angle_) return angle_->polygon();
  else if (cobb_angle_) return cobb_angle_->polygon();
  else if (poly_) return poly_->polygon();
  else return QPolygonF();
}

QVector<QPoint> GraphicsItem::points() const {
  if (smart_curve_) {
    return smart_curve_->points();
  }

  return {};
}

GraphicsItem::Type GraphicsItem::getType() const {
  return type_;
}

double GraphicsItem::length() const {
  if (type_ == Type::Line) {
    return line_->line().length();
  }
  else if (type_ == Type::Poly) {
    return poly_->polygon().length();
  }
  else if (type_ == Type::SmartCurve) {
    return smart_curve_->points().length();
  }

  return 0.0;
}

bool GraphicsItem::isSelected() const {
  return selected_;
}

bool GraphicsItem::isItemUnderMouse() const {
  return item_->isUnderMouse();
}

bool GraphicsItem::isCreated() const {
  return created_;
}

bool GraphicsItem::isUnderPos(const QPointF& p) const {
  if (type_ == Type::Line) {
    return line_->isUnderPos(p) || item_->isUnderMouse();
  }
  else if (type_ == Type::Ellipse || type_ == Type::Angle || type_ == Type::CobbAngle || type_ == Type::Poly || type_ == Type::SmartCurve) {
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
  else if (type_ == Type::CobbAngle) {
    auto poly = cobb_angle_->polygon();
    return poly.count() > 2 && dist(poly[0], poly[1]) > 7 && dist(poly[2], poly[3]) > 7;
  }
  else if (type_ == Type::Poly) {
    auto poly = poly_->polygon();
    return poly.count() > 2;
  }
  else if (type_ == Type::SmartCurve) {
    auto points = smart_curve_->points();
    return smart_curve_->points().size() >= 2 && dist(points[0], points[1]) > 7;
  }

  return false;
}

bool GraphicsItem::isPartUnderPos(const QPointF& coord) const {
  const auto r = base_touch_radius / scale_factor_;
  if (type_ == Type::Line) {
    auto data = line_->line();
    if (dist(coord, data.p1()) < r || dist(coord, data.p2()) < r) {
      return true;
    }
  }
  else if (type_ == Type::Ellipse) {
    auto rect = ellipse_->rect();
    for (auto pt : { rect.topLeft(), rect.topRight(), rect.bottomLeft(), rect.bottomRight() }) {
      if (dist(coord, pt) < r) {
        return true;
      }
    }
  }
  else if (type_ == Type::Angle) {
    auto poly = angle_->polygon();
    for (int k = 0; k < poly.count(); ++k) {
      if (dist(coord, poly[k].toPoint()) < r) {
        return true;
      }
    }
  }
  else if (type_ == Type::CobbAngle) {
    auto poly = cobb_angle_->polygon();
    for (int k = 0; k < poly.count(); ++k) {
      if (dist(coord, poly[k].toPoint()) < r) {
        return true;
      }
    }
  }
  else if (type_ == Type::Poly) {
    auto poly = poly_->polygon();
    for (int k = 0; k < poly.count(); ++k) {
      if (dist(coord, poly[k].toPoint()) < r) {
        return true;
      }
    }
  }
  else if (type_ == Type::SmartCurve) {
    auto points = smart_curve_->points();
    for (int k = 0; k < points.count(); ++k) {
      if (dist(coord, points[k]) < r) {
        return true;
      }
    }
  }

  return false;
}

void GraphicsItem::setFixedColor(const QColor& color) {
  selected_color_ = color;
  default_color_ = color;
  calibrated_color_ = color;
  updateColors();
}

void GraphicsItem::setSelectedColor(const QColor& color) {
  selected_color_ = color;
  updateColors();
}

void GraphicsItem::setDefaultColor(const QColor& color) {
  default_color_ = color;
  updateColors();
}

void GraphicsItem::setCalibratedColor(const QColor& color) {
  calibrated_color_ = color;
  updateColors();
}

void GraphicsItem::setPolygon(const QPolygonF& poly) {
  if (angle_) {
    if (angle_->polygon().size() <= 2 && poly.size() > 2) {
      anchor_index_ = 2;
    }

    angle_->setPolygon(poly);
  }
  else if (cobb_angle_) {
    cobb_angle_->setPolygon(poly);
    if (poly.size() == 1) anchor_index_ = 1;
    else if (poly.size() == 3) anchor_index_ = 3;
    else anchor_index_ = -1;
  }
  else if (poly_) {
    poly_->setPolygon(poly);
  }
}

void GraphicsItem::setPoints(const QVector<QPoint>& points) {
  if (smart_curve_) {
    smart_curve_->setPoints(points);
  }
}

void GraphicsItem::setCalibrationCoef(std::optional<qreal> coef) {
  calib_coef_ = coef;
  updateCaption();
}

void GraphicsItem::setGradient(const cv::Mat_<double>& gradient) {
  if (smart_curve_) {
    smart_curve_->setGradient(gradient);
  }
}

void GraphicsItem::setScaleFactor(float scale_factor) {
  scale_factor_ = scale_factor;
  item_->setFont(QFont("Arial", 10 / scale_factor_));

  if (line_) line_->setScaleFactor(scale_factor_);
  if (ellipse_) ellipse_->setScaleFactor(scale_factor_);
  if (cobb_angle_) cobb_angle_->setScaleFactor(scale_factor_);
  if (angle_) angle_->setScaleFactor(scale_factor_);
  if (poly_) poly_->setScaleFactor(scale_factor_);
  if (smart_curve_) smart_curve_->setScaleFactor(scale_factor_);

  updateCaption();
  updateColors();
  update();
}

void GraphicsItem::setSelected(bool selected) {
  selected_ = selected;
  updateColors();
}

void GraphicsItem::setCreated(bool created, std::optional<int> r) {
  created_ = created;
  if (r) {
    rotation = r.value();
  }
}

void GraphicsItem::checkPartUnderPos(const QPointF& coord) {
  const auto r = base_touch_radius / scale_factor_;
  if (type_ == Type::Line) {
    auto data = line_->line();
    if (dist(coord, data.p1()) < r) {
      line_->setPartUnderMouse(0);
    }
    else if (dist(coord, data.p2()) < r) {
      line_->setPartUnderMouse(1);
    }
    else line_->setPartUnderMouse(-1);
  }
  else if (type_ == Type::Ellipse) {
    auto rect = ellipse_->rect();
    QPointF points[] = { rect.bottomLeft(), rect.topLeft(), rect.topRight(), rect.bottomRight() };
    for (int k = 0; k < 4; ++k) {
      if (dist(coord, points[k]) < r) {
        ellipse_->setPartUnderMouse(k);
        return;
      }
    }

    ellipse_->setPartUnderMouse(-1);
  }
  else if (type_ == Type::Angle) {
    auto poly = angle_->polygon();
    for (int k = 0; k < poly.count(); ++k) {
      if (dist(coord, poly[k].toPoint()) < r) {
        angle_->setPartUnderMouse(k);
        return;
      }
    }

    angle_->setPartUnderMouse(-1);
  }
  else if (type_ == Type::CobbAngle) {
    auto poly = cobb_angle_->polygon();
    for (int k = 0; k < poly.count(); ++k) {
      if (dist(coord, poly[k].toPoint()) < r) {
        cobb_angle_->setPartUnderMouse(k);
        return;
      }
    }

    cobb_angle_->setPartUnderMouse(-1);
  }
  else if (type_ == Type::Poly) {
    auto poly = poly_->polygon();
    for (int k = 0; k < poly.count(); ++k) {
      if (dist(coord, poly[k].toPoint()) < r) {
        poly_->setPartUnderMouse(k);
        return;
      }
    }

    poly_->setPartUnderMouse(-1);
  }
  else if (type_ == Type::SmartCurve) {
    auto points = smart_curve_->points();
    for (int k = 0; k < points.count(); ++k) {
      if (dist(coord, points[k]) < r) {
        smart_curve_->setPartUnderMouse(k);
        return;
      }
    }

    smart_curve_->setPartUnderMouse(-1);
  }
}

bool GraphicsItem::checkSelection(const QPointF& pos) {
  highlighted_ = isUnderPos(pos);
  
  updateColors();

  return highlighted_;
}

void GraphicsItem::updateColors() {
  if (highlighted_ || selected_) {
    item_->setBackgroundColor(QColor(80, 80, 0, 200));
    if (line_) line_->setPen(QPen(selected_color_, 2 / scale_factor_));
    if (ellipse_) ellipse_->setPen(QPen(selected_color_, 2 / scale_factor_));
    if (cobb_angle_) cobb_angle_->setPen(QPen(selected_color_, 2 / scale_factor_));
    if (angle_) angle_->setPen(QPen(selected_color_, 2 / scale_factor_));
    if (poly_) poly_->setPen(QPen(selected_color_, 2 / scale_factor_));
    if (smart_curve_) smart_curve_->setPen(QPen(selected_color_, 2 / scale_factor_));
  }
  else if (calib_coef_) {
    item_->setBackgroundColor(QColor(0, 88, 0, 200));
    if (line_) line_->setPen(QPen(calibrated_color_, 2 / scale_factor_));
    if (ellipse_) ellipse_->setPen(QPen(calibrated_color_, 2 / scale_factor_));
    if (cobb_angle_) cobb_angle_->setPen(QPen(calibrated_color_, 2 / scale_factor_));
    if (angle_) angle_->setPen(QPen(calibrated_color_, 2 / scale_factor_));
    if (poly_) poly_->setPen(QPen(calibrated_color_, 2 / scale_factor_));
    if (smart_curve_) smart_curve_->setPen(QPen(calibrated_color_, 2 / scale_factor_));
  }
  else {
    item_->setBackgroundColor(QColor(80, 80, 0, 200));
    if (line_) line_->setPen(QPen(default_color_, 2 / scale_factor_));
    if (ellipse_) ellipse_->setPen(QPen(default_color_, 2 / scale_factor_));
    if (cobb_angle_) cobb_angle_->setPen(QPen(default_color_, 2 / scale_factor_));
    if (angle_) angle_->setPen(QPen(default_color_, 2 / scale_factor_));
    if (poly_) poly_->setPen(QPen(default_color_, 2 / scale_factor_));
    if (smart_curve_) smart_curve_->setPen(QPen(default_color_, 2 / scale_factor_));
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

    item_->setPos((line.p1().x() > line.p2().x() ? line.p1() : line.p2()) + QPointF(8, -10) / scale_factor_);
  }
  else if (type_ == Type::Ellipse) {
    auto rect = ellipse_->rect();
    auto y = (rect.bottomRight().y() + rect.topRight().y()) / 2;
    if (calib_coef_) {
      item_->setPlainText(
        "Area=" + QString::number(area_.value_or(0) * calib_coef_.value()) + " mm\n"
        "Min=" + QString::number(min_.value_or(0)) + "  Max=" + QString::number(max_.value_or(0)) + "\n"
        "Avg=" + QString::number(avg_.value_or(0)));
    }
    else {
      item_->setPlainText(
        "Area=" + QString::number(area_.value_or(0)) + " px\n"
        "Min=" + QString::number(min_.value_or(0)) + "  Max=" + QString::number(max_.value_or(0)) + "\n"
        "Avg=" + QString::number(avg_.value_or(0)));
    }

    item_->setPos(QPointF(qMax(rect.left(), rect.right()), y) + QPointF(11, -10) / scale_factor_);
  }
  else if (type_ == Type::Angle) {
    auto poly = angle_->polygon();
    auto rightest_pt = poly.first();
    for (int k = 1; k < poly.count(); ++k) {
      if (poly[k].x() > rightest_pt.x()) {
        rightest_pt = poly[k];
      }
    }

    item_->setPlainText("Angle: " + QString::number(angle_->angle()));
    item_->setPos(rightest_pt + QPointF(11, -10) / scale_factor_);
  }
  else if (type_ == Type::CobbAngle) {
    auto poly = cobb_angle_->polygon();
    auto rightest_pt = poly.first();
    for (int k = 1; k < poly.count(); ++k) {
      if (poly[k].x() > rightest_pt.x()) {
        rightest_pt = poly[k];
      }
    }

    item_->setPlainText("Angle: " + QString::number(cobb_angle_->angle()));
    item_->setPos(rightest_pt + QPointF(11, -10) / scale_factor_);
  }
  else if (type_ == Type::Poly) {
    auto poly = poly_->polygon();
    auto rightest_pt = poly.first();
    for (int k = 1; k < poly.count(); ++k) {
      if (poly[k].x() > rightest_pt.x()) {
        rightest_pt = poly[k];
      }
    }

    if (poly.size() < 3) {
      item_->setVisible(false);
    }
    else {
      item_->setVisible(true);
      item_->setPos(rightest_pt + QPointF(11, -10) / scale_factor_);
      if (calib_coef_) {
        item_->setPlainText(
          "Area=" + QString::number(area_.value_or(0) * calib_coef_.value()) + "mm P=" + QString::number(perimeter_.value_or(0) * calib_coef_.value()) + "mm\n"
          "Min=" + QString::number(min_.value_or(0)) + "  Max=" + QString::number(max_.value_or(0)) + "\n"
          "Avg=" + QString::number(avg_.value_or(0)));
      }
      else {
        item_->setPlainText(
          "Area=" + QString::number(area_.value_or(0)) + +"px P=" + QString::number(perimeter_.value_or(0)) + "px\n"
          "Min=" + QString::number(min_.value_or(0)) + "  Max=" + QString::number(max_.value_or(0)) + "\n"
          "Avg=" + QString::number(avg_.value_or(0)));
      }
    }
  }
  else if (type_ == Type::SmartCurve) {
    auto points = smart_curve_->points();
    auto rightest_pt = points.first();
    for (int k = 1; k < points.count(); ++k) {
      if (points[k].x() > rightest_pt.x()) {
        rightest_pt = points[k];
      }
    }

    if (points.size() < 2) {
      item_->setVisible(false);
    }
    else {
      item_->setVisible(true);
      item_->setPos(rightest_pt + QPointF(11, -10) / scale_factor_);

      // TODO:
      if (calib_coef_) {
        item_->setPlainText(QString::number(points.length() * calib_coef_.value(), 'f', 2) + " mm");
      }
      else {
        item_->setPlainText(QString::number(points.length(), 'f', 2) + " px");
      }
    }
  }

  update();
}

void GraphicsItem::mousePressEvent(const QPointF& pos) {
  if (!created_)
    return;

  anchor_index_ = -1;

  const auto r = base_touch_radius / scale_factor_;
  if (type_ == Type::Line) {
    auto line = line_->line();
    if (dist(pos, line.p1()) < r) {
      anchor_index_ = 0;
    }
    else if (dist(pos, line.p2()) < r) {
      anchor_index_ = 1;
    }
  }
  else if (type_ == Type::Ellipse) {
    auto rect = ellipse_->rect();
    if (dist(rect.bottomLeft(), pos) < r) anchor_index_ = 0;
    else if (dist(rect.topLeft(), pos) < r) anchor_index_ = 1;
    else if (dist(rect.topRight(), pos) < r) anchor_index_ = 2;
    else if (dist(rect.bottomRight(), pos) < r) anchor_index_ = 3;
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
      if (dist(poly[k], pos) < r) {
        anchor_index_ = k;
        break;
      }
    }
  }
  else if (type_ == Type::CobbAngle) {
    auto poly = cobb_angle_->polygon();
    for (int k = 0; k < poly.count(); ++k) {
      if (dist(poly[k], pos) < r) {
        anchor_index_ = k;
        break;
      }
    }
  }
  else if (type_ == Type::Poly) {
    auto poly = poly_->polygon();
    for (int k = poly.count() - 1; k >= 0; --k) {
      if (dist(poly[k], pos) < r) {
        anchor_index_ = k;
        break;
      }
    }
  }
  else if (type_ == Type::SmartCurve) {
    auto points = smart_curve_->points();
    for (int k = points.count() - 1; k >= 0; --k) {
      if (dist(points[k], pos) < r) {
        anchor_index_ = k;
        break;
      }
    }
  }
}

void GraphicsItem::mouseReleaseEvent(const QPointF& pos) {
  if (type_ == Type::SmartCurve) {
    if (anchor_index_ >= 0) {
      smart_curve_->setPoint(anchor_index_, pos.toPoint());
    }
  }
}

void GraphicsItem::mouseMoveEvent(const QPointF& pos, const cv::Mat& image) {
  if (type_ == Type::Line) {
    auto line = line_->line(); 
    if (anchor_index_ == 0) line.setP1(pos);
    else line.setP2(pos);
    
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
  }
  else if (type_ == Type::Angle) {
    if (anchor_index_ >= 0) {
      angle_->setPoint(anchor_index_, pos);
    }
    else {
      angle_->setPoint(angle_->polygon().count() - 1, pos);
    }
  }
  else if (type_ == Type::CobbAngle) {
    if (anchor_index_ >= 0) {
      cobb_angle_->setPoint(anchor_index_, pos, created_);
    }
  }
  else if (type_ == Type::Poly) {
    if (anchor_index_ >= 0) {
      poly_->setPoint(anchor_index_, pos);
    }
    else {
      poly_->setPoint(poly_->polygon().count() - 1, pos);
    }

    auto poly = poly_->polygon();
    auto rect = poly.boundingRect();
    int sum = 0, count = 0, max = 0, min = INT_MAX;
    int left = qMax<int>(0, qMin(rect.x(), rect.x() + rect.width()));
    int bottom = qMax<int>(0, qMin(rect.y(), rect.y() + rect.height()));
    int right = qMin(left + qAbs<int>(rect.width()), image.cols - 1), top = qMin<int>(bottom + qAbs(rect.height()), image.rows - 1);
    for (int i = left; i <= right; ++i) {
      for (int j = bottom; j <= top; ++j) {
        if (poly.containsPoint(QPointF(i, j), Qt::FillRule::OddEvenFill)) {
          auto px = image.at<cv::Vec2b>(j, i);
          min = qMin<int>(px[0], min);
          max = qMax<int>(px[0], max);
          sum += px[0];
          count += 1;
        }
      }
    }

    int p = dist(poly.first(), poly.back());
    for (int k = 0; k < poly.size()-1; ++k) {
      p += dist(poly[k], poly[k + 1]);
    }

    min_ = min;
    max_ = max;
    area_ = count;
    perimeter_ = p;
    avg_ = double(sum) / count;
  }
  else if (type_ == Type::SmartCurve) {
    if (anchor_index_ >= 0) {
      smart_curve_->setPoint(anchor_index_, pos.toPoint());
    }
  }

  updateCaption();
}

void GraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) {
  Q_UNUSED(o);
  Q_UNUSED(w);
}

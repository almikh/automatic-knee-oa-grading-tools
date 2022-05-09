#pragma once

enum Rotation {
  Rotate90_CW = 90,
  Rotate90_CCW = -90,
  Rotate180 = 180,
};

enum Transformation {
  HFlip = 0,
  VFlip = 1
};

struct ViewportState {
  double scale = 1.0;
  QPointF position;
};

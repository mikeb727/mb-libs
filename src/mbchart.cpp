#include "mbchart.h"

namespace ChartTools {

AutoScaleMode operator|(AutoScaleMode lv, AutoScaleMode rv) {
  return AutoScaleMode(int(lv) | int(rv));
}

ChartPoint::ChartPoint() : x(0), y(0) {}

ChartPoint::ChartPoint(double xVal, double yVal) : x(xVal), y(yVal) {}

DataSet::DataSet(int setSize) : currentSize(0), maxSize(setSize) {
  points = new ChartPoint[maxSize];
}

DataSet::~DataSet() { delete points; }

void DataSet::addPoint(double x, double y) {
  if (currentSize < maxSize) {
    points[currentSize] = ChartPoint(x, y);
    currentSize++;
    totalPoints++;
  } else {
    for (int i = 1; i < maxSize; i++) { // start at 1, since comparing to prev
      points[i - 1] = points[i];
    }
    points[currentSize] = ChartPoint(x, y);
    totalPoints++;
  }
}

void DataSet::addPoint2(double y) {
  if (currentSize < maxSize) {
    points[currentSize] = ChartPoint(totalPoints, y);
    currentSize++;
    totalPoints++;
  } else {
    for (int i = 1; i < maxSize; i++) {
      points[i - 1] = points[i];
    }
    points[currentSize - 1] = ChartPoint(totalPoints, y);
    totalPoints++;
  }
}

double DataSet::averageLastPoints(int numPoints) const {
  double sum = 0;
  int pointCount = 0;
  for (int i = currentSize - 1; i > currentSize - numPoints - 1; i--) {
    if (i < 0) {
      break;
    }
    sum += points[i].y;
    pointCount++;
  }
  if (pointCount == 0) {
    pointCount++;
  }
  return sum / (double)pointCount;
}

std::vector<float> DataSet::getPointCoords() const {
  std::vector<float> result;
  for (int p = 0; p < currentSize; ++p) {
    result.push_back(points[p].x);
    result.push_back(points[p].y);
  }
  return result;
}

LineChart::LineChart(GraphicsTools::Window *parent, DataSet *dataset, int drawX,
                     int drawY, int width, int height)
    : parentWindow(parent), data(dataset), drawPosX(drawX), drawPosY(drawY),
      drawWidth(width), drawHeight(height), scaleModesX(NONE),
      scaleModesY(NONE) {

  borderColor = GraphicsTools::Colors::White;
  gridColor = GraphicsTools::ColorRgba({255, 200, 200, 63});
  backgroundColor = GraphicsTools::Colors::Black;
  dataColor = GraphicsTools::Colors::Green;

  showLabels = false;

  xAxisMin = 0;
  xAxisMax = 60;
  yAxisMin = 0;
  yAxisMax = 10;
  xGridInterval = 10;
  xGridOffset = 0;
  yGridInterval = 1;
  yGridOffset = 0;

  ySoftMarginBottom = 0.05;
  ySoftMarginTop = 0.15;

  yAxisSmMin =
      mapY((yAxisMin * ySoftMarginBottom) + (yAxisMax * (1 - ySoftMarginTop)));
  yAxisSmMax =
      mapY((yAxisMin * (1 - ySoftMarginBottom)) + (yAxisMax * ySoftMarginTop));
}

LineChart::~LineChart() {}

int LineChart::mapX(double sourceVal) {
  double sourceMin = xAxisMin;
  double sourceMax = xAxisMax;
  double targetMin = drawPosX;
  double targetMax = drawPosX + drawWidth;
  return targetMin + (((targetMax - targetMin) * (sourceVal - sourceMin)) /
                      (sourceMax - sourceMin));
}

int LineChart::mapY(double sourceVal) {
  double sourceMin = yAxisMin;
  double sourceMax = yAxisMax;
  double targetMin = drawPosY;
  double targetMax = drawPosY + drawHeight;
  return targetMin + (((targetMax - targetMin) * (sourceVal - sourceMin)) /
                      (sourceMax - sourceMin));
}

void LineChart::autoScaleX() {
  double maxPointVal = data->getPoint(0).x;
  double minPointVal = data->getPoint(0).x;
  for (int i = 1; i < data->getCurrentSize(); i++) {
    if (data->getPoint(i).x > maxPointVal) {
      maxPointVal = data->getPoint(i).x;
    }
    if (data->getPoint(i).x < minPointVal) {
      minPointVal = data->getPoint(i).x;
    }
  }
  double newXMin = xAxisMin;
  double newXMax = xAxisMax;
  if (scaleModesX | SCALE_MIN) {
    newXMin = minPointVal;
  }
  if (scaleModesX | SCALE_MAX) {
    newXMax = maxPointVal;
  }
  setAxes(newXMin, newXMax, yAxisMin, yAxisMax);
}

void LineChart::autoScaleY() {
  double maxPointVal = data->getPoint(0).y;
  double minPointVal = data->getPoint(0).y;
  for (int i = 1; i < data->getCurrentSize(); i++) {
    if (data->getPoint(i).y > maxPointVal) {
      maxPointVal = data->getPoint(i).y;
    }
    if (data->getPoint(i).y < minPointVal) {
      minPointVal = data->getPoint(i).y;
    }
  }
  double newYMin = yAxisMin;
  double newYMax = yAxisMax;
  if (scaleModesY | SCALE_MIN) {
    newYMin = yAxisSmMin - ((yAxisMax - yAxisMin) * ySoftMarginBottom);
  }
  if (scaleModesY | SCALE_MAX) {
    newYMax = yAxisSmMax + ((yAxisMax - yAxisMin) * ySoftMarginTop);
  }
  yAxisSmMax = std::fmax(maxPointVal, 0);
  yAxisSmMin = std::fmin(minPointVal, 0);
  setAxes(xAxisMin, xAxisMax, newYMin, newYMax);
}

void LineChart::draw() {

  // Scaling
  autoScaleX();
  autoScaleY();

  // Y-grid (vertical lines)
  for (int _x = xAxisMin + std::fmod(-xAxisMin, xGridInterval); _x <= xAxisMax;
       _x += xGridInterval) {
    if (_x > xAxisMin && _x < xAxisMax) {
      parentWindow->drawLine(gridColor, 1, mapX(_x), drawPosY + drawHeight,
                             mapX(_x), drawPosY);
    }
  }
  // X-grid (horizontal lines)
  for (int _y = yAxisMin + std::fmod(-yAxisMin, yGridInterval); _y <= yAxisMax;
       _y += yGridInterval) {
    if (_y > yAxisMin && _y < yAxisMax) {
      parentWindow->drawText(std::to_string(_y), labelFont,
                             GraphicsTools::Colors::White, drawPosX - 40,
                             mapY(_y) - 10, 100);
      parentWindow->drawLine(gridColor, 1, drawPosX, mapY(_y),
                             drawPosX + drawWidth, mapY(_y));
    }
  }

  // Data
  if (data->getCurrentSize() == 1) {
    parentWindow->drawCircle(dataColor, mapX(data->getPoint(0).x),
                             mapY(data->getPoint(0).y), 8);
  } else if (data->getCurrentSize() > 1) {
    std::vector<float> points = data->getPointCoords();
    for (int p = 0; p < points.size()/2; ++p) {
      points[2*p] = mapX(points[2*p]);
      points[2*p+1] = mapY(points[2*p+1]);
    }
    parentWindow->drawMultiLine(dataColor, 2, data->getCurrentSize(),
                                points.data());
    // for (int i = 0; i < data->getCurrentSize() - 1; i++) {
    //   parentWindow->drawLine(
    //       dataColor, 2, mapX(data->getPoint(i).x), mapY(data->getPoint(i).y),
    //       mapX(data->getPoint(i + 1).x), mapY(data->getPoint(i + 1).y));
    // }
  }

  // X-axis (y = 0)
  int xAxisPosition = mapY(0);
  if (xAxisPosition > drawPosY && xAxisPosition < drawPosY + drawHeight) {
    parentWindow->drawLine(GraphicsTools::Colors::Grey, 4, drawPosX, mapY(0),
                           drawPosX + drawWidth, mapY(0));
  }
  // X-axis
  parentWindow->drawLine(borderColor, 4, drawPosX, drawPosY + drawHeight,
                         drawPosX + drawWidth, drawPosY + drawHeight);
  // X-axis parallel
  parentWindow->drawLine(borderColor, 4, drawPosX, drawPosY,
                         drawPosX + drawWidth, drawPosY);
  // Y-axis
  parentWindow->drawLine(borderColor, 4, drawPosX, drawPosY + drawHeight,
                         drawPosX, drawPosY);
  // Y-axis parallel
  parentWindow->drawLine(borderColor, 4, drawPosX + drawWidth,
                         drawPosY + drawHeight, drawPosX + drawWidth, drawPosY);
}

void LineChart::setAxes(double xMin, double xMax, double yMin, double yMax) {
  xAxisMin = xMin;
  xAxisMax = xMax;
  yAxisMin = yMin;
  yAxisMax = yMax;
}
void LineChart::setGrid(double xInt, double xOff, double yInt, double yOff) {
  xGridInterval = xInt;
  xGridOffset = xOff;
  yGridInterval = yInt;
  yGridOffset = yOff;
}

void LineChart::setScaleModes(AutoScaleMode x, AutoScaleMode y) {
  scaleModesX = x;
  scaleModesY = y;
}

void LineChart::setLabelFont(GraphicsTools::Font *f) { labelFont = f; }
} // namespace ChartTools

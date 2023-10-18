#ifndef CHARTS_H
#define CHARTS_H

#include "mbgfx.h"
#include "window.h"

#include <cmath>
#include <iostream>
#include <string>

namespace ChartTools {

enum AutoScaleMode {
  NONE = 0x00,
  SCALE_MIN = 0x01,    // auto-scale the axis lower bound to data
  SCALE_MAX = 0x02,    // auto-scale the axis upper bound to data
  PRESERVE_ZERO = 0x04 // force the axis range to contain zero (prevents axis
                       // truncation)
};


AutoScaleMode operator|(AutoScaleMode lv, AutoScaleMode rv);

struct ChartPoint {
  double x;
  double y;
  ChartPoint();
  ChartPoint(double, double);
};

class DataSet {
private:
  int maxSize;
  int currentSize;
  int totalPoints;
  ChartPoint *points;

public:
  DataSet(int setSize);
  ~DataSet();
  void addPoint(double x, double y);
  void addPoint2(double y); // for continuous-time values
  void clear() { currentSize = 0; };
  const ChartPoint getPoint(int index) const { return points[index]; };

  int getMaxSize() const { return maxSize; };
  int getCurrentSize() const { return currentSize; };
  double averageLastPoints(int numPoints) const;
  std::vector<float> getPointCoords() const;
};

class LineChart {
private:
  GraphicsTools::Window *parentWindow;
  int drawPosX;
  int drawPosY;
  int drawWidth;
  int drawHeight;

  GraphicsTools::ColorRgba borderColor;
  GraphicsTools::ColorRgba gridColor;
  GraphicsTools::ColorRgba backgroundColor;
  GraphicsTools::ColorRgba dataColor;

  // y-axis labels
  bool showLabels;
  GraphicsTools::Font *labelFont;

  double xAxisMin;
  double xAxisMax;
  double yAxisMin;
  double yAxisMax;
  double xGridInterval;
  double xGridOffset;
  double yGridInterval;
  double yGridOffset;

  double ySoftMarginBottom;
  double ySoftMarginTop;

  double yAxisSmMin;
  double yAxisSmMax;

  DataSet *data;

  // maps a data value to a position on the window.
  int mapX(double sourceVal);
  int mapY(double sourceVal);

  void autoScaleX();
  void autoScaleY();

  AutoScaleMode scaleModesX;
  AutoScaleMode scaleModesY;

public:
  LineChart(GraphicsTools::Window *parent, DataSet *data, int drawX, int drawY,
            int width, int height);
  ~LineChart();
  void draw();
  void setAxes(double xMin, double xMax, double yMin, double yMax);
  void setLabelFont(GraphicsTools::Font *f);
  void setLabels(bool labelsOn) { showLabels = labelsOn; };
  void setGrid(double xInt, double xOff, double yInt, double yOff);
  void setScaleModes(AutoScaleMode x, AutoScaleMode y);
};

} // namespace ChartTools

#endif // CHARTS_H

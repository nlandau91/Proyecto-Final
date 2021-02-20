#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "edge.h"

class Triangle
{
public:
    Triangle(int x0, int y0, int x1, int y1, int x2, int y2);
    Triangle(const Edge &e01, const Edge &e02);
    Edge e01, e02;
    double angle;
    bool compare(const Triangle &triangle, double ang_dif = 5.0, double edg_rat = 0.1)const;
};

#endif // TRIANGLE_H

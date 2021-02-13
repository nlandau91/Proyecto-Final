#ifndef EDGE_H
#define EDGE_H

#include "common.h"

class Edge
{
public:
    Edge(int x1, int y1, double ang1, int x2, int y2, double ang2);

    int x1, y1, x2, y2;
    double dist, angle, alpha, beta;

};

#endif // EDGE_H

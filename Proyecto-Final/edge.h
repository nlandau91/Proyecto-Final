#ifndef EDGE_H
#define EDGE_H

#include "common.h"

class Edge
{
public:
    Edge(int x1, int y1, int x2, int y2);

    int x1, y1, x2, y2;
    double dist;
    double angle;
};

#endif // EDGE_H

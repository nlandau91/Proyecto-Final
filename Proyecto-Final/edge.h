#ifndef EDGE_H
#define EDGE_H

#include "common.h"

class Edge
{
public:
    Edge(int x1, int y1, float ang1, int x2, int y2, float ang2);

    bool compare(const Edge &e, double dist_dif = 10, double angle_dif = 10) const;

    int x1, y1, x2, y2;
    float dist, angle, alpha, beta;

};

#endif // EDGE_H

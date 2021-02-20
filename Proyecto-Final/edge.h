#ifndef EDGE_H
#define EDGE_H

#include "common.h"

class Edge
{
public:
    Edge();
    Edge(int x1, int y1, int x2, int y2);

    double angle(const Edge &e)const;

    bool compare(const Edge& e, double ratio)const;

    double ratio(const Edge& e)const;

    int x, y;
    double length;

};

#endif // EDGE_H

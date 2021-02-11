#include "edge.h"

Edge::Edge(int x1, int y1, int x2, int y2)
{
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    this->dist = std::pow((x1 - x2), 2) + pow(y1 - y2, 2);
    this->angle = atan2(y1 - y2, x1 - x2);

}

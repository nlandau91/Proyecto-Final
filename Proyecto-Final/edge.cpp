#include "edge.h"
#include <QDebug>

Edge::Edge()
{
    this->x = 0;
    this->y = 0;

    this->length = 0;
}
Edge::Edge(int x1, int y1, int x2, int y2)
{
    this->x = x1 - x2;
    this->y = y1 = y2;
    this->length = std::sqrt(std::pow(x, 2) + std::pow(y, 2));

}

double Edge::angle(const Edge &e)const
{
    return std::atan2(x*e.y-y*e.x,x*e.x+y*e.y) * 180 / M_PI;
}

bool Edge::compare(const Edge &e, double rat)const
{
    return length * rat < e.length && e.length * rat < length;
}

double Edge::ratio(const Edge &e)const
{

}

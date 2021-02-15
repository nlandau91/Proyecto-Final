#include "edge.h"
#include <QDebug>

Edge::Edge(int x1, int y1, float ang1, int x2, int y2, float ang2)
{
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    this->dist = std::sqrt(std::pow((x1 - x2), 2) + pow(y1 - y2, 2));
    this->angle = atan2(y1 - y2, x1 - x2) * 180.0 / M_PI;
    this->alpha = ang1 - angle; // ver como hacer mejor
    this->beta = ang2 - angle; //ver como hacer mejor
}

bool Edge::compare(const Edge &e, double dist_dif, double angle_dif) const
{
    bool similares = false;
    //    similares = std::abs(dist - e.dist) < dist_dif &&
    //            std::abs(alpha - e.alpha) < angle_dif &&
    //            std::abs(beta - e.beta) < angle_dif;
    similares = std::abs(dist - e.dist) < dist_dif &&
            std::abs(angle - e.angle) < angle_dif;
    return similares;
}

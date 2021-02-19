#include "edge.h"
#include <QDebug>

Edge::Edge(int x1, int y1, float ang1, int x2, int y2, float ang2)
{
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    this->dist = std::sqrt(std::pow((x1 - x2), 2) + std::pow(y1 - y2, 2));

    this->angle = std::atan2(y1 - y2, x1 - x2) * 180.0 / M_PI;
    this->alpha = std::fmod(((ang1 * 180.0 / M_PI) - angle),360); // ver como hacer mejor
    if(this->alpha < 0)
    {
        this->alpha += 360;
    }
    this->beta = std::fmod(((ang2 * 180.0 / M_PI) - angle),360); //ver como hacer mejor
    if(this->beta < 0)
    {
        this->beta += 360;
    }

}

bool isBetween(float start, float end, float mid)
{
    float norm_end = (end - start) < 0.0f ? end - start + 360.0f : end - start;
    float norm_mid = (mid - start) < 0.0f ? mid - start + 360.0f : mid - start;
    return (norm_mid < norm_end);
}

bool Edge::compare(const Edge &e, double dist_dif, double angle_dif) const
{
    bool similares = false;
    similares = std::abs(dist/e.dist) < dist_dif &&
            std::abs(angle/e.angle) < angle_dif;

//    similares = std::abs(dist - e.dist) < dist_dif &&
//            isBetween(alpha - angle_dif, alpha + angle_dif, e.alpha) &&
//            isBetween(beta - angle_dif, beta + angle_dif, e.beta);
    return similares;
}

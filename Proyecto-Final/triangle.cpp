#include "triangle.h"

#include <QDebug>

Triangle::Triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
    this->e01 = Edge(x0,y0,x1,y1);
    this->e02 = Edge(x0,y0,x2,y2);
    this->angle = e01.angle(e02);
}

Triangle::Triangle(const Edge &e01, const Edge &e02)
{
    this->e01 = e01;
    this->e02 = e02;
    this->angle = this->e01.angle(e02);
}

float _getComponentX(float alpha)
   {
     return cos(alpha * M_PI / 180);
   }

   float _getComponentY(float alpha)
   {
     return sin(alpha * M_PI / 180);
   }

   float _getAngleRatioX(float alpha, float beta)
   {
     return _getComponentX(alpha) * _getComponentX(beta);
   }

   float _getAngleRatioY(float alpha, float beta)
   {
     return _getComponentY(alpha) * _getComponentY(beta);
   }

   float getSimilarity(float alpha, float beta)
   {
     float ratioX = _getAngleRatioX(alpha, beta);
     float ratioY = _getAngleRatioY(alpha, beta);
     float ratio = ratioX + ratioY; // Ratio is between 1 and -1
     return (ratio + 1) / 2; // Return a value between 0 and 1
   }

   float getWeightedValue(float alpha, float beta, float value)
   {
     float similarity = getSimilarity(alpha, beta);
     return similarity * value;
   }

bool Triangle::compare(const Triangle &triangle, double ang_dif, double edg_rat)const
{
    //comparamos angulos
    double min_simil = getSimilarity(0,ang_dif);
    bool comp_ang = getSimilarity(angle, triangle.angle) >= min_simil;
    bool comp11 = e01.length * edg_rat < triangle.e01.length && triangle.e01.length * edg_rat < e01.length;
    bool comp22 = e02.length * edg_rat < triangle.e02.length && triangle.e02.length * edg_rat < e02.length;
//    qDebug() << "comp_ang: " << comp_ang;
//    qDebug() << "comp11: " << comp11;
//    qDebug() << "comp22: " << comp22;
    return comp_ang & comp11 && comp22;

}

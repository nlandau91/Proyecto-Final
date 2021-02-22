#include "triangle.h"

#include <QDebug>

Triangle::Triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
    this->e01 = Edge(x0,y0,x1,y1);
    this->e02 = Edge(x0,y0,x2,y2);
    this->e12 = Edge(x1,y1,x2,y2);
    this->angle = e01.angle(e02);
}

Triangle::Triangle(const Edge &e01, const Edge &e02, const Edge &e12)
{
    this->e01 = e01;
    this->e02 = e02;
    this->e12 = e12;
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
    bool comp1 = false;
    bool comp2 = false;
    bool comp3 = false;
    if(comp_ang)
    {
        comp1 = e01.compare(triangle.e01,edg_rat);
        if(comp1) //gaste 01
        {
            comp2 = e02.compare(triangle.e02,edg_rat);
            if(comp2) //gaste 01 y 02
            {
                comp3 = e12.compare(triangle.e12,edg_rat);
            }
            else
            {
                comp2 = e02.compare(triangle.e12,edg_rat);
                if(comp2) //gaste 01 y 12
                {
                    comp3 = e12.compare(triangle.e02,edg_rat);
                }
            }
        }
        else
        {
            comp1 = e01.compare(triangle.e02,edg_rat);
            if(comp1) //gaste 02
            {
                comp2 = e02.compare(triangle.e01,edg_rat);
                if(comp2) //gaste 01 y 02
                {
                    comp3 = e12.compare(triangle.e12,edg_rat);
                }
                else
                {
                    comp2 = e02.compare(triangle.e12,edg_rat);
                    if(comp2) // gaste 02 y 12
                    {
                        comp3 = e12.compare(triangle.e01,edg_rat);
                    }
                }
            }
            else
            {
                comp1 = e01.compare(triangle.e12,edg_rat);
                if(comp1) //gaste 12
                {
                    comp2 = e02.compare(triangle.e01,edg_rat);
                    if(comp2) //gaste 01 y 12
                    {
                        comp3 = e12.compare(triangle.e02,edg_rat);
                    }
                    else
                    {
                        comp2 = e02.compare(triangle.e02,edg_rat);
                        if(comp2) //gaste 02 y 12
                        {
                            comp3 = e12.compare(triangle.e01,edg_rat);
                        }
                    }
                }
            }
        }
    }
    return comp_ang && comp1 && comp2 && comp3;
}

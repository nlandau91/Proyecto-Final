#ifndef TRIANGLE_H
#define TRIANGLE_H


class Triangle
{
public:
    Triangle();
    double x0,y0,x1,y1,x2,y2; //vert pos. x0,y0 is the root
    double d01,d02,d12; //distances
    double a01,a02; //angles relative to horizontal
};

#endif // TRIANGLE_H

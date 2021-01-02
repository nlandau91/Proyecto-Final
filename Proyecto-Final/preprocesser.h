#ifndef FINGERPRINTENHANCER_H
#define FINGERPRINTENHANCER_H

#include <opencv2/opencv.hpp>

namespace fp
{

//clase que realiza la mejora de las imagenes de huellas dactilares
class Enhancer
{
public:
    enum EnhancementMethod
    {
        NONE,
        SKELETONIZE,
        GABORFILTERS
    };

    Enhancer();

    //mejora la imagen de una huella dactilar
    static cv::Mat enhance(cv::Mat &src, EnhancementMethod method);
};
}
#endif // FINGERPRINTENHANCER_H

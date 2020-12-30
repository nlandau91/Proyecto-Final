#ifndef FINGERPRINTENHANCER_H
#define FINGERPRINTENHANCER_H

#include <opencv2/opencv.hpp>

//clase que realiza la mejora de las imagenes de huellas dactilares
class FingerprintEnhancer
{
public:
    enum EnhancementMethod
    {
        NONE,
        SKELETONIZE,
        GABORFILTERS
    };

    FingerprintEnhancer();

    //mejora la imagen de una huella dactilar
    static cv::Mat enhance(cv::Mat &src, EnhancementMethod method);
};

#endif // FINGERPRINTENHANCER_H

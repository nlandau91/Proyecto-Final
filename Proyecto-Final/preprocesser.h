#ifndef FINGERPRINTENHANCER_H
#define FINGERPRINTENHANCER_H

#include <opencv2/opencv.hpp>

namespace fp
{

//clase que realiza el preprocesamiento de las huellas dactilares
class Preprocesser
{
public:
    enum EnhancementMethod
    {
        GABORFILTERS
    };
    enum ThinningMethod
    {
        ZHANGSUEN,
        MORPHOLOGICAL,
        GUOHALL,
    };

    Preprocesser();

    //realiza el preprocesamiento de una huella dactilar par adecuarla a la extraccion de features
    static cv::Mat preprocess(cv::Mat &src, EnhancementMethod enhancement_method, ThinningMethod thinning_method);

    //mejora una huella dactilar
    static cv::Mat enhance(cv::Mat &src, EnhancementMethod enhancement_method);
    //esqueletiza una huella dactilar
    static cv::Mat thin(cv::Mat &src, ThinningMethod thinning_method);
    //elimina el fondo de la imagen para solo quedarnos con el area de interes
    static cv::Mat roi_mask(cv::Mat &original, cv::Mat &preprocessed);
};
}
#endif // FINGERPRINTENHANCER_H

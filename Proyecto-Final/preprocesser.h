#ifndef FINGERPRINTENHANCER_H
#define FINGERPRINTENHANCER_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QMetaEnum>
#include <QDebug>

namespace fp
{

//clase que realiza el preprocesamiento de las huellas dactilares
class Preprocesser : public QObject
{
    Q_OBJECT
public:
    enum EnhancementMethod
    {
        NONE,
        GABOR,
    };
    Q_ENUM(EnhancementMethod)
    enum ThinningMethod
    {
        ZHANGSUEN,
        MORPH,
        GUOHALL,
    };
     Q_ENUM(ThinningMethod)

    Preprocesser();

    //realiza el preprocesamiento de una huella dactilar par adecuarla a la extraccion de features
    static cv::Mat preprocess(cv::Mat &src, EnhancementMethod enhancement_method = GABOR, ThinningMethod thinning_method = ZHANGSUEN, bool roi_masking = false);

    //mejora una huella dactilar
    static cv::Mat enhance(cv::Mat &src, EnhancementMethod enhancement_method);
    //esqueletiza una huella dactilar
    static cv::Mat thin(cv::Mat &src, ThinningMethod thinning_method);
    //elimina el fondo de la imagen para solo quedarnos con el area de interes
    static cv::Mat roi_mask(cv::Mat &original, cv::Mat &preprocessed);
};
}
#endif // FINGERPRINTENHANCER_H

#ifndef FINGERPRINTENHANCER_H
#define FINGERPRINTENHANCER_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QMetaEnum>

namespace fp
{

//clase que realiza el preprocesamiento de las huellas dactilares
class Preprocesser : public QObject
{
    Q_OBJECT
public:
    enum EnhancementMethod
    {
        ENH_NONE,
        ENH_GABOR,
    };
    enum ThinningMethod
    {
        THI_NONE,
        THI_ZHANGSUEN,
        THI_MORPH,
        THI_GUOHALL,
    };

    Preprocesser();

    //realiza el preprocesamiento de una huella dactilar par adecuarla a la extraccion de features
    static cv::Mat preprocess(cv::Mat &src, EnhancementMethod enhancement_method = ENH_GABOR, ThinningMethod thinning_method = THI_ZHANGSUEN, bool roi_masking = false);

    //normaliza una imagen para que tenga media y var elegidas
    //entrada: CV_32FC1
    //salida: CV_32FC1
    static cv::Mat normalize(cv::Mat &src, float req_mean, float req_var);

    //calcula la roi de una imagen
    //supone que la imagen ya esta normalizada
    //entrada: CV_32FC1
    //salida: CV_8UC1
    static cv::Mat get_roi(cv::Mat &src,int block_size = 16, float threshold_ratio = 0.2);

    //mejora una imagen con el metodo elegido
    //entrada CV_32FC1
    //salida CV_32FC1
    static cv::Mat enhance(cv::Mat &src, EnhancementMethod enhancement_method);

    //esqueletiza una huella dactilar
    //entrada: CV_8UC1
    //salida: CV_8UC1
    static cv::Mat thin(cv::Mat &src, ThinningMethod thinning_method);
};
}
#endif // FINGERPRINTENHANCER_H

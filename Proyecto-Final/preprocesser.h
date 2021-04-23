#ifndef PREPROCESSER_H
#define PREPROCESSER_H

#include "preprocessed.h"

#include "utils.h"

namespace fp
{

/*!
 * La clase Preprocesser realiza el adecuamiento de la imagen de una huella
 * dactilar para poder ser analizada posteriormente
 */
class Preprocesser
{
public:

    int enhancement_method;
    int thinning_method;
    float norm_req_mean;
    float norm_req_var;
    int blk_freq;
    int blk_orient;
    int blk_mask;
    int blk_sze;
    float roi_threshold_ratio;
    bool segment;
    float gabor_kx;
    float gabor_ky;

    Preprocesser();

    /*!
     * \brief preprocess realiza el preprocesamiento de la imagen para adecuarla al analisis
     * \param src imagen que sera preprocesada
     * \return estructura que contiene las imagenes del preprocesamiento
     */
    Preprocessed preprocess(const cv::Mat &src);
};
}
#endif // PREPROCESSER_H

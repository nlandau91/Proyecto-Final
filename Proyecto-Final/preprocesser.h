#ifndef PREPROCESSER_H
#define PREPROCESSER_H

#include "common.h"
#include "utils.h"

namespace fp
{

//clase que realiza el preprocesamiento de las huellas dactilares
class Preprocesser
{
public:



    int enhancement_method;
    int thinning_method;
    float norm_req_mean;
    float norm_req_var;
    int blk_sze;
    float roi_threshold_ratio;
    bool segment;

    /*!
     * \brief inicializa con valores por defecto
     * enhacer de gabor
     * thinning de zhangsuen
     */
    Preprocesser() : enhancement_method(GABOR),thinning_method(ZHANGSUEN),
        norm_req_mean(100.0), norm_req_var(100.0),
        blk_sze(16),roi_threshold_ratio(0.2),
        segment(true){}

    Preprocesser(int enhancement_method, int thinning_method)
        :enhancement_method(enhancement_method),thinning_method(thinning_method),
          norm_req_mean(100.0), norm_req_var(100.0),
          blk_sze(16),roi_threshold_ratio(0.2),
          segment(true){}

    /*!
     * \brief preprocess realiza el preprocesamiento de una huella dactilar para
     * adecuara a la extraccion de caracteristicas. Esta funcion se encarga de llamar
     * al resto de las funciones en el orden correcto.
     * \param src huella dactilar en formato CV_8UC1
     * \return devuelve la imagen luego de pasar por el pipeline de preprocesamiento, CV_8UC1
     */
    //cv::Mat preprocess(cv::Mat &src);


    Preprocessed preprocess(cv::Mat &src);

    /*!
     * \brief normalize normaliza una imagen para que tenga la media y var elegidas
     * \param src imagen a normalizar
     * \param req_mean media requerida, 0.0 por defecto
     * \param req_var var requerida, 1.0 por defecto
     * \param mask mascara opcional
     * \return
     */
    cv::Mat normalize(cv::Mat &src, float req_mean = 0.0, float req_var = 1.0,const cv::_InputOutputArray &mask = cv::noArray());

    /*!
     * \brief calcula y devuelve una mascara representando la roi de la imagen
     * utilizando cambios en la variacion local
     * \param src imagen que se calculara la roi, se supone normalizada y CV_32FC1
     * \param block_size tamano del bloque donde se calculara la variacion local
     * \param threshold_ratio afecta al threshold calculado, un valor mayor enmascara mas cantidad
     * \return la mascara que representa el roi de la imagen, CV_8UC1
     */
    cv::Mat get_roi(cv::Mat &src,int block_size = 16, float threshold_ratio = 0.2);

    /*!
     * \brief esqueletiza una huella dactilar
     * \param src imagen de la huella a esqueletizar, CV_8UC1
     * \param thinning_method metodo a utilizar
     * \return la imagen esqueletizada, CV_8UC1
     */
    cv::Mat thin(cv::Mat &src, int thinning_method);
};
}
#endif // PREPROCESSER_H

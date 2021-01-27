#ifndef ANALYZER_H
#define ANALYZER_H

#include "common.h"
#include "utils.h"
namespace fp
{
/*!
 * \brief La clase Analyzer se encarga de realizar el analisis y extraccion de caracteristicas
 * de una huella dactilar
 */
class Analyzer
{
public:

    /*!
     * \brief Analyzer
     */
    Analyzer() :
        l2_features_method(HARRIS),keypoint_threshold(130),
        descriptor_method(ORB),max_match_dist(80){}

    Analyzer(int keypoint_method, int keypoint_threshold, int descriptor_method, int matcher_method, int max_match_dist)
        : l2_features_method(keypoint_method), keypoint_threshold(keypoint_threshold),
          descriptor_method(descriptor_method),matcher_method(matcher_method),
          max_match_dist(max_match_dist){}

    /*!
     * \brief analize analiza la huella dactilar preprocesada
     * \param preprocessed estructura que contiene el preprocesamiento hecho a la huella dactilar
     * \return devuelve una estructura que contiene el analisis hecho
     */
    Analysis analize(const Preprocessed &preprocessed);

private:
    int l2_features_method;
    int keypoint_threshold;
    int descriptor_method;
    int matcher_method;
    int max_match_dist;
    int l1_features_method;
    float poincare_tol;
    int blk_sze;

    /*!
     * \brief find_l1_features
     * \param pre
     * \return
     */
    std::vector<cv::KeyPoint> find_l1_features(const Preprocessed &pre);

    /*!
     * \brief find_l2_features
     * \param pre
     * \return
     */
    std::vector<cv::KeyPoint> find_l2_features(const Preprocessed &pre);

    /*!
     * \brief calcular_descriptors calcula los decriptores para los puntos claves
     * \param src
     * \param keypoints
     * \return
     */
    cv::Mat calcular_descriptors(const cv::Mat &src, std::vector<cv::KeyPoint> &keypoints);
};
}
#endif // ANALYZER_H

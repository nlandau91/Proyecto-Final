#ifndef ANALYZER_H
#define ANALYZER_H

#include "common.h"
#include <opencv2/opencv.hpp>
namespace fp
{
//clase para realizar el analisis y extraccion de caracteristicas de una huella dactilar
//se asume que las imagenes de huellas ya estan preprocesadas
class Analyzer
{
public:
    struct Analysis
    {
        cv::Mat fingerprint;
        std::vector<cv::KeyPoint> l1_features;
        std::vector<cv::KeyPoint> l2_features;
        cv::Mat descriptors;
    };

    Analyzer() :
        l2_features_method(HARRIS),keypoint_threshold(130),
        descriptor_method(ORB),max_match_dist(80){}

    Analyzer(int keypoint_method, int keypoint_threshold, int descriptor_method, int matcher_method, int max_match_dist)
        : l2_features_method(keypoint_method), keypoint_threshold(keypoint_threshold),
          descriptor_method(descriptor_method),matcher_method(matcher_method),
          max_match_dist(max_match_dist){}

    /*!
     * \brief analize analiza la huella dactilar
     * \param src huella dactilar a analizar, se supone ya preprocesada
     * \return analisis de la huella
     */
    Analysis analize(cv::Mat &src);

private:
    int l2_features_method;
    int keypoint_threshold;
    int descriptor_method;
    int matcher_method;
    int max_match_dist;
    int l1_features_method;
    cv::Mat mask;
    cv::Mat orient;
    float poincare_tol;
    int blk_sze;

    /*!
     * \brief find_l1_features calcula las posiciones donde se encuentran features de nivel 1
     * esto es, loops, deltas o whorls.
     * \param src imagen de la huella, se supone ya preprocesada
     * \return vector con las posiciones de los features encontrados.
     */
    std::vector<cv::KeyPoint> find_l1_features(cv::Mat &src);

    /*!
     * \brief find_l2_features calcula las posiciones donde se encuentram features de nivel 2
     * esto es, las minucias conocidas como bifurcaciones y terminaciones de crestas
     * \param src imagen de la huella, se supone ya preprocesada
     * \return vector con los puntos encontrados
     */
    std::vector<cv::KeyPoint> find_l2_features(cv::Mat &src);

    cv::Mat calcular_descriptors(cv::Mat &src, std::vector<cv::KeyPoint> keypoints);
};
}
#endif // ANALYZER_H

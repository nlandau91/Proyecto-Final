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
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
    };

    Analyzer() :
        keypoint_method(HARRIS),keypoint_threshold(130),descriptor_method(ORB),max_match_dist(80){}

    Analyzer(int keypoint_method, int keypoint_threshold, int descriptor_method, int matcher_method, int max_match_dist)
        : keypoint_method(keypoint_method), keypoint_threshold(keypoint_threshold),
          descriptor_method(descriptor_method),matcher_method(matcher_method),
          max_match_dist(max_match_dist){}

    /*!
     * \brief analize analiza la huella dactilar
     * \param src huella dactilar a analizar, se supone ya preprocesada
     * \return analisis de la huella
     */
    Analysis analize(cv::Mat &src);

private:
    int keypoint_method;
    int keypoint_threshold;
    int descriptor_method;
    int matcher_method;
    int max_match_dist;

    std::vector<cv::KeyPoint> calcular_keypoints(cv::Mat &src);
    cv::Mat calcular_descriptors(cv::Mat &src, std::vector<cv::KeyPoint> keypoints);
};
}
#endif // ANALYZER_H

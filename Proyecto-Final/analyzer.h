#ifndef FINGERPRINTANALYZER_H
#define FINGERPRINTANALYZER_H

#include <opencv2/opencv.hpp>
namespace fp
{
//clase para realizar el analisis y extraccion de caracteristicas de una huella dactilar
//se asume que las imagenes de huellas ya estan preprocesadas
class Analyzer
{
public:
    enum KeypointMethod
    {
        HARRIS,
    };
    enum DescriptorMethod
    {
        ORB,
    };

    struct Analysis
    {
        cv::Mat fingerprint;
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
    };


    Analyzer();

    //analiza la huella digital
    static Analysis analize(cv::Mat &src, KeypointMethod keypoint_method = HARRIS, DescriptorMethod descriptor_method = ORB);

};
}
#endif // FINGERPRINTANALYZER_H

#ifndef PREPROCESSED_H
#define PREPROCESSED_H
#include <opencv2/core/mat.hpp>
namespace fp
{
/*!
 * \brief El struct Preprocessed contiene las imagenes resultado del preprocesamiento
 * de una huella dactilar
 */
struct Preprocessed
{
    cv::Mat original;
    cv::Mat normalized;
    cv::Mat roi;
    cv::Mat orientation;
    cv::Mat frequency;
    cv::Mat filtered;
    cv::Mat thinned;
    cv::Mat grayscale;
    cv::Mat result;
};
}
#endif // PREPROCESSED_H

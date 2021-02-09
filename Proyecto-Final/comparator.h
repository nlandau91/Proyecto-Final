#ifndef COMPARATOR_H
#define COMPARATOR_H

#include "common.h"

namespace fp{

class Comparator
{
public:
    Comparator();

    /*!
     * \brief compare compara los descriptores de consulta contra una lista de descriptores de comparacion
     * \param descriptors ddescriptores de consulta
     * \param lista_descriptores descriptores de comparacion
     * \return un valor numerico correspondiente a la distancia entre los descriptores.
     *  Mientras mas bajo, mas se asemejan los descriptores de consulta con los de comparacion
     */
    bool compare(const cv::Mat &query_descriptors, const std::vector<cv::Mat> &train_descriptors_list, double threshold = 0.1);

    bool compare(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, double threshold = 0.1);

    bool compare_edges(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors);

    int matcher_method;
};
}
#endif // COMPARATOR_H

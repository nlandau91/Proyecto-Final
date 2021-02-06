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
    int compare(const cv::Mat &descriptors, const std::vector<cv::Mat> &lista_descriptores);

    int matcher_method;
};
}
#endif // COMPARATOR_H

#ifndef COMPARATOR_H
#define COMPARATOR_H

#include "common.h"

namespace fp{

class Comparator
{
public:
    Comparator();

    //compara los descriptores de consulta contra una lista de descriptores de comparacion
    //devuelve un score de distancia, mientras mas bajo, mas se asemejan los descriptores de consulta con los de comparacion
    static int compare(cv::Mat &descriptors, std::vector<cv::Mat> &lista_descriptores);
};
}
#endif // COMPARATOR_H

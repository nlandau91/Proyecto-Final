#ifndef COMPARATOR_H
#define COMPARATOR_H

#include "common.h"
#include "edge.h"
#include "triangle.h"
#include "fingerprinttemplate.h"

namespace fp{

class Comparator
{
public:
    Comparator();

    /*!
     * \brief compare compara dos template de huellas dactilares y decide si se corresponden al mismo dedo
     * \param query_template template de consulta
     * \param train_template template de verificacion
     * \param threshold umbral de decicion. Mientras mas alto, mas estricto. Rango [0,1]
     * \return true si se considera que los templates corresponden a un mismo dedo
     */
    bool compare(const FingerprintTemplate &query_template, const FingerprintTemplate &train_template);

    double compare(const FingerprintTemplate &query_template, const FingerprintTemplate &train_template, bool flag);

    /*!
     * \brief compare compara un template con una lista de templates y decide si se corresponde a alguno de ellos
     * La idea es que la lista de templates contenga distintos templates del mismo dedo
     * \param query_template template de consulta
     * \param train_templates templates de verificacion
     * \return true si se considera que el template de consulta corresponde a alguno de la lista
     */
    bool compare(const FingerprintTemplate &query_template, const std::vector<FingerprintTemplate> &train_templates);



    int matcher_method;
    double match_threshold;
    bool edge_matching;
    double edge_dist;
    double edge_angle;
    double triangle_max_edge;
    double triangle_min_edge;
    double ransac_threshold;
};
}
#endif // COMPARATOR_H

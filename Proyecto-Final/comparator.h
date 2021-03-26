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
     * \brief compare compara dos template de huellas dactilares y devuelve un score de similitud
     * \param query_template template de consulta
     * \param train_template template de verificacion
     * \return score de similitud, entre 0.0 y 1.0
     */
    double compare(const FingerprintTemplate &query_template, const FingerprintTemplate &train_template) const;

    /*!
     * \brief compare
     * \param query_template
     * \param train_templates
     * \return
     */
    double compare(const FingerprintTemplate &query_template, const std::vector<FingerprintTemplate> &train_templates) const;


    int matcher_method;
    double match_threshold;
    bool edge_matching;
    double edge_dist;
    double edge_angle;
    double triangle_max_edge;
    double triangle_min_edge;
    double ransac_threshold;
    int ransac_iter;
    double ransac_conf;
    int ransac_transform;
    double median_threshold;
    bool sing_compare;

};
}
#endif // COMPARATOR_H

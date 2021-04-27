#ifndef COMPARATOR_H
#define COMPARATOR_H


#include "fingerprinttemplate.h"

namespace fp
{

/*!
 * La clase Comparator se encarga de realizar la comparacion entre
 * dos templates de huellas dactilares.
 */
class Comparator
{
public:
    /*!
     * \brief Comparator constructor
     */
    Comparator();

    /*!
     * \brief compare compara dos templates de huellas dactilares y devuelve un score de similitud
     * \param query_template template de consulta
     * \param train_template template de verificacion
     * \return score de similitud, entre 0.0 y 1.0
     */
    double compare(const FingerprintTemplate &query_template, const FingerprintTemplate &train_template) const;

    int matcher_method;
    double match_threshold;
    double ransac_threshold;
    int ransac_iter;
    double ransac_conf;
    int ransac_model;
    double median_threshold;
    bool sing_compare;

};
}
#endif // COMPARATOR_H

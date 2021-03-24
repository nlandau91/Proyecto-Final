#ifndef STATS_H
#define STATS_H
#include "common.h"
#include "utils.h"

namespace fp{

/*!
 * \brief get_mean devuelve la media de un arreglo de valores entre 0 y 1
 * \param scores arreglo de valores
 * \param ignore_extremes indica si ingnorar los valores 0.00 y 1.00
 * \return
 */
double get_mean(const std::vector<double> &scores, bool ignore_extremes = true);

/*!
 * \brief get_low_pcnt devuelve el mayor valor encontrado dentro de la porcion inferior del arreglo indicado
 * \param scores arreglo de valores
 * \param thresh valor entre 0 y 1 indicando porcentaje del arreglo desde el comienzo
 * \return mayor valor incluido en la porcion del umbral
 */
double get_low_pcnt(std::vector<double> scores, double thresh);

/*!
 * \brief get_high_pcnt devuelve el menor valor encontrado dentro de la porcion superior del arreglo indicada
 * \param scores arreglo de valores
 * \param thresh valor entre 0 y 1 indicando el porcentaje del arreglo desde el final hacia atras
 * \return menor valor incluido en la porcion del umbral
 */
double get_high_pcnt(std::vector<double> scores, double thresh);

/*!
 * \brief get_eer calcula el eer de los arreglos de scores
 * //el mismo indica en que porcentaje de los arreglos los valores del primer arreglo se asemejan
 * lo suficiente a los del arreglo 2 avanzando en sentido inverso
 * \param scores1 arreglo de valores 1
 * \param scores2 arreglo de valores 2
 * \return equal error rate calculado
 */
double get_eer(std::vector<double> scores1, std::vector<double> scores2);
}

#endif // STATS_H

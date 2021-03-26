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
 * \return vector con el primer valor indicando el eer%, el segundo valor indicando que score se encuentra en ese %
 */
std::vector<double> get_eer(std::vector<double> scores1, std::vector<double> scores2);

/*!
 * \brief get_frr calcula la tasa de falsos positivos
 * \param scores arreglo de valores entre 0 y 1
 * \param threshold valor minimo que debe tener un score para ser considerado positivo
 * \return tasa de falsos positivos, entre 0 y 1
 */
double get_far(const std::vector<double> &scores, double threshold);

/*!
 * \brief get_far calcula la tasa de falsos negativos
 * \param scores arreglo de valores entre 0 y 1
 * \param threshold valor minimo que debe tener un score para ser considerado positivo
 * \return tasa de falsos negativos, entre 0 y 1
 */
double get_frr(const std::vector<double> &scores, double threshold);
}

#endif // STATS_H

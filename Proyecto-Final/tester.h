#ifndef TESTER_H
#define TESTER_H

#include "database.h"
#include "preprocesser.h"
#include "analyzer.h"
#include "comparator.h"

namespace fp
{
/*!
 * \brief The Tester class implementa algunos test a medida
 */
class Tester
{
public:
    /*!
     * \brief Tester inicializador vacio, es necesario setear
     * el preprocesador, analizado y comparador, y tener cargada la db
     */
    Tester();
    /*!
     * \brief load_database realiza una carga de la base de datos en lote
     * las imagenes deben llamarse <id>_*.tif
     * \param db referencia a la base de datos
     * \param path directorio donde se encuentran las imagenes
     */
    void load_database(Database &db, const QString &path);
    /*!
     * \brief test_far realiza la comparacion de todas las huellas contra todas
     * las que corresponden a un id distinto
     * \param db
     * \return devuelve arreglo con todos los scores
     */
    std::vector<double> test_far(const Database &db);
    /*!
     * \brief test_frr realiza la comparacion de todas las huellas contra
     * todas las que corresponden al mismo id
     * \param db
     * \return devuelve arreglo con todos los scores
     */
    std::vector<double> test_frr(const Database &db);

    void perform_tests(const std::vector<std::vector<double>> &params_list, Database &db);

    Preprocesser preprocesser;
    Analyzer analyzer;
    Comparator comparator;
};
}
#endif // TESTER_H

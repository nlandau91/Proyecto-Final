#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"

#include <opencv2/opencv.hpp>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>
namespace fp
{

class Database
{
public:
    Database();

    /*!
     * \brief init inicializa la base de datos
     */
    void init();

    /*!
     * \brief ingresar_descriptor guarda el descriptor en disco y lo ingresa a la base de datos
     * \param descriptors descriptores
     * \param id identificador
     */
    void ingresar_descriptor(cv::Mat &descriptors, const QString &id);

    /*!
     * \brief obtener_lista_descriptores devuelve devuelve una lista con los discriptores de las muestras asociadas al id
     * \param id identificador que se buscara para obtener los descriptores
     * \return lista con los descriptores asociadas a id
     */
    std::vector<cv::Mat> obtener_lista_descriptores(const QString &id);

    /*!
     * \brief obtener_lista_id devuelve una lista con los id dentro de la base de datos
     * \return lista con los id dentro de la base de datos
     */
    std::vector<QString> obtener_lista_id();

private:
    QSqlDatabase db;
};
}
#endif // DATABASE_H

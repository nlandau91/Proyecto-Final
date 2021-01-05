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
    //inicializa la base de datos
    void init();
    //guarda el descriptor en disco y lo ingresa a la base de datos
    void ingresar_descriptor(cv::Mat &descriptors, const QString &id);
    //devuelve los descriptores de las muestras correspondientes al id
    std::vector<cv::Mat> obtener_lista_descriptores(const QString &id);
    //devuelve una lista con los id dentro de la base de datos
    std::vector<QString> obtener_lista_id();
private:
    QSqlDatabase db;
    void setup_database();
};
}
#endif // DATABASE_H

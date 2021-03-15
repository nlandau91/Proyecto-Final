#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include "utils.h"
#include "fingerprinttemplate.h"

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

    void ingresar_template(const FingerprintTemplate &fp_template, const QString &id);

    std::vector<FingerprintTemplate> recuperar_template(const QString &id);

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

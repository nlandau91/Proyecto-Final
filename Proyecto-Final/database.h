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

    /*!
     * \brief ingresar_template ingresa un template a la base de datos
     * \param fp_template template a ingresar
     * \param id id asociado al template
     */
    void ingresar_template(const FingerprintTemplate &fp_template, const QString &id) const;

    /*!
     * \brief recuperar_template devuelve los templates asociados al id
     * \param id por el cual se buscaran templates
     * \return lista de templates asociados al id
     */
    std::vector<FingerprintTemplate> recuperar_template(const QString &id) const;

    /*!
     * \brief obtener_lista_id devuelve una lista con los id dentro de la base de datos
     * \return lista con los id dentro de la base de datos
     */
    std::vector<QString> obtener_lista_id() const;

private:
    QSqlDatabase db;
};
}
#endif // DATABASE_H

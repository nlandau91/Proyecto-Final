#include "database.h"
#include <QDebug>

using namespace fp;

bool Database::init()
{
    //database setup
    const QString DRIVER("QSQLITE");
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db = QSqlDatabase::addDatabase(DRIVER);
        //db.setDatabaseName(":memory:");
        db.setDatabaseName("../db/fingerprint.db");
        if(!db.open())
        {
            qWarning() << "ERROR: " << db.lastError();
            return false;
        }
        //creamos una tabla
        QSqlQuery query("CREATE TABLE people (id INTEGER, sample INTEGER, template_path TEXT, PRIMARY KEY(id, sample))");
        if(!query.isActive())
        {
            //no se pudo crear la table, posiblemente porque ya existia
            //qWarning() << "ERROR: " << query.lastError().text();
        }
    }
    return true;
}


Database::Database()
{

}

bool Database::ingresar_template(const FingerprintTemplate &fp_template, const QString &id)
{
    //armamos el path
    QDir qdir = QDir::current();
    QString template_path = qdir.path()+"/../db/fp_templates/"+id;
    qdir.mkpath(template_path);

    //vemos que numero de muestra estamos por ingresar
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM people WHERE id=:id");
    query.bindValue(":id",id);
    if(!query.exec())
    {
        qWarning() << "ERROR: " << query.lastError().text();
        return false;
    }
    int sample = 0;
    if(query.first())
    {
        sample = query.value(0).toInt();
    }
    //guardamos el archivo en disco armando el nombre
    template_path = template_path + "/" + QString::number(sample);
    fp_template.serialize(template_path.toStdString());

    //ingresamos los descriptores a la base de datos
    query.prepare("INSERT INTO people(id, sample, template_path) VALUES (:id, :sample, :template_path)");
    query.bindValue(":id",id);
    query.bindValue(":sample",sample);
    query.bindValue(":template_path", template_path);

    if(!query.exec())
    {
        qWarning() << "ERROR: " << query.lastError().text();
        return false;
    }
    return true;
}

std::vector<FingerprintTemplate> Database::recuperar_template(const QString &id) const
{
    std::vector<FingerprintTemplate> lista_templates;

    QSqlQuery query;
    query.prepare("SELECT template_path FROM people WHERE id=:id");
    query.bindValue(":id",id);
    if(!query.exec())
    {
        qWarning() << "ERROR: " << query.lastError().text();
    }
    while(query.next())
    {
        QString path = query.value(0).toString();
        FingerprintTemplate fp_template(path.toStdString());
        lista_templates.push_back(fp_template);
    }

    //devolvemos los templates
    return lista_templates;
}

std::vector<QString> Database::obtener_lista_id() const
{
    std::vector<QString> lista_id;
    QSqlQuery query;
    query.prepare("SELECT DISTINCT id FROM people");
    if(!query.exec())
    {

    }
    while(query.next())
    {
        QString id = query.value(0).toString();
        lista_id.push_back(id);
    }
    return lista_id;
}

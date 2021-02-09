#include "database.h"
#include <QDebug>

namespace fp
{

void Database::init()
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
        }
        //creamos una tabla
        QSqlQuery query("CREATE TABLE people (id INTEGER, sample INTEGER, descriptor_path TEXT, keypoint_path TEXT, PRIMARY KEY(id, sample))");
        if(!query.isActive())
        {
            //no se pudo crear la table, posiblemente porque ya existia
            //qWarning() << "ERROR: " << query.lastError().text();
        }
    }
}


Database::Database()
{

}

void Database::ingresar_descriptores(const cv::Mat &descriptors, const QString &id)
{
    if(!descriptors.empty())
    {
        //armamos el path
        QDir qdir = QDir::current();
        QString descriptor_path = qdir.path()+"/../db/descriptors/"+id;
        qdir.mkpath(descriptor_path);

        //vemos que numero de muestra estamos por ingresar
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) FROM people WHERE id=:id");
        query.bindValue(":id",id);
        if(!query.exec())
        {
            qWarning() << "ERROR: " << query.lastError().text();
        }
        int sample = 0;
        if(query.first())
        {
            sample = query.value(0).toInt();
        }
        //guardamos el archivo en disco armando el nombre
        //descriptor_path = descriptor_path + "/" + QString::number(sample)+".tif";
        //cv::imwrite(descriptor_path.toStdString(), descriptors);
        descriptor_path = descriptor_path + "/" + QString::number(sample);
        serialize_mat(descriptor_path.toStdString(),descriptors);
        //ingresamos los descriptores a la base de datos
        query.prepare("INSERT INTO people(id, sample, descriptor_path) VALUES (:id, :sample, :path)");
        query.bindValue(":id",id);
        query.bindValue(":path", descriptor_path);
        query.bindValue(":sample",sample);
        if(!query.exec())
        {
            qWarning() << "ERROR: " << query.lastError().text();
        }

    }
}

void Database::ingresar_keypoints(const cv::Mat &keypoints, const QString &id)
{
    if(!keypoints.empty())
    {
        //armamos el path
        QDir qdir = QDir::current();
        QString keypoint_path = qdir.path()+"/../db/keypoints/"+id;
        qdir.mkpath(keypoint_path);

        //vemos que numero de muestra estamos por ingresar
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) FROM people WHERE id=:id");
        query.bindValue(":id",id);
        if(!query.exec())
        {
            qWarning() << "ERROR: " << query.lastError().text();
        }
        int sample = 0;
        if(query.first())
        {
            sample = query.value(0).toInt();
        }
        //guardamos el archivo en disco armando el nombre
        //keypoint_path = keypoint_path + "/" + QString::number(sample)+".tif";
        //cv::imwrite(descriptor_path.toStdString(), descriptors);
        keypoint_path = keypoint_path + "/" + QString::number(sample);
        serialize_mat(keypoint_path.toStdString(),keypoints);
        //ingresamos los descriptores a la base de datos
        query.prepare("INSERT INTO people(id, sample, descriptor_path) VALUES (:id, :sample, :path)");
        query.bindValue(":id",id);
        query.bindValue(":path", keypoint_path);
        query.bindValue(":sample",sample);
        if(!query.exec())
        {
            qWarning() << "ERROR: " << query.lastError().text();
        }

    }
}

void Database::ingresar_fp(const Analysis &fp, const QString &id)
{
    if(!fp.descriptors.empty() && !fp.keypoints.empty())
    {
        //armamos el path
        QDir qdir = QDir::current();
        QString descriptor_path = qdir.path()+"/../db/descriptors/"+id;
        qdir.mkpath(descriptor_path);

        QString keypoint_path = qdir.path()+"/../db/keypoints/"+id;
        qdir.mkpath(keypoint_path);

        //vemos que numero de muestra estamos por ingresar
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) FROM people WHERE id=:id");
        query.bindValue(":id",id);
        if(!query.exec())
        {
            qWarning() << "ERROR: " << query.lastError().text();
        }
        int sample = 0;
        if(query.first())
        {
            sample = query.value(0).toInt();
        }
        //guardamos el archivo en disco armando el nombre
        descriptor_path = descriptor_path + "/" + QString::number(sample);
        serialize_mat(descriptor_path.toStdString(),fp.descriptors);

        keypoint_path = keypoint_path + "/" + QString::number(sample);
        serialize_mat(keypoint_path.toStdString(),fp.keypoints);
        //ingresamos los descriptores a la base de datos
        query.prepare("INSERT INTO people(id, sample, descriptor_path, keypoint_path) VALUES (:id, :sample, :descriptor_path, :keypoint_path)");
        query.bindValue(":id",id);
        query.bindValue(":descriptor_path", descriptor_path);
        query.bindValue(":keypoint_path", keypoint_path);
        query.bindValue(":sample",sample);
        if(!query.exec())
        {
            qWarning() << "ERROR: " << query.lastError().text();
        }
    }
}

std::vector<cv::Mat> Database::recuperar_descriptores(const QString &id)
{
    std::vector<cv::Mat> lista_descriptores;

    QSqlQuery query;
    query.prepare("SELECT descriptor_path FROM people WHERE id=:id");
    query.bindValue(":id",id);
    if(!query.exec())
    {
        qWarning() << "ERROR: " << query.lastError().text();
    }
    while(query.next())
    {
        QString path = query.value(0).toString();
        //cv::Mat descriptores = cv::imread(path.toStdString(),cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        cv::Mat descriptores;
        serialize_mat(path.toStdString(),descriptores);
        lista_descriptores.push_back(descriptores);
    }


    //devolvemos los descriptores
    return lista_descriptores;
}
std::vector<QString> Database::obtener_lista_id()
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
}

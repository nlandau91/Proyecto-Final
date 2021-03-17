#include "tester.h"

#include <QDebug>

using namespace fp;
Tester::Tester()
{

}

//arma una base de datos para realizar pruebas a partir de una muestra de huellas
//se espera que las huellas se encuentren de la siguiente manera
//en el directorio raiz dos directorios FVC2000 y FVC2002
//dentro de cada uno de esos directorios, cuatro directorios DB1_B, DB2_B, DB3_B, DB4_B
//dentro de cada uno de esos directorios, las huellas llamadas A_B.tif, donde A indica un dedo y B una muestra de ese dedo
//por lo tanto FVC2000->DB1_B->101_1.tif y FVC2000->DB1_B->101_2.tif corresponden a dos muestras del mismo dedo
void Tester::load_database(fp::Database &db)
{
    //cargamos las carpetas que contienen huellas
    std::vector<QString> paths;
    paths.push_back("../res/FVC2000/DB1_B");
    paths.push_back("../res/FVC2000/DB2_B");
    paths.push_back("../res/FVC2000/DB3_B");
    paths.push_back("../res/FVC2000/DB4_B");
    paths.push_back("../res/FVC2002/DB1_B");
    paths.push_back("../res/FVC2002/DB2_B");
    paths.push_back("../res/FVC2002/DB3_B");
    paths.push_back("../res/FVC2002/DB4_B");
    int id = 0;
    for(const QString &path : paths)
    {
        std::cout << path.toStdString() << std::endl;
        //para cada carpeta, cargamos todas las huellas
        QDir directory(path);
        QStringList fileNames = directory.entryList(QStringList() << "*.tif",QDir::Files);
        for(const QString &fileName : fileNames)
        {
            std::cout << fileName.toStdString() << std::endl;
            //para cada huella, la procesamos y la ingresamos
            std::string abs_path = (path+"/"+fileName).toStdString();
            cv::Mat src = cv::imread(abs_path,cv::IMREAD_GRAYSCALE);
            std::cout << "1"<< std::endl;
            if(!src.empty())
            {
                //preprocesamos la imagen
                fp::Preprocessed pre = preprocesser.preprocess(src);
                //obtenemos los descriptores
                fp::FingerprintTemplate fp_template = analyzer.analize(pre);
                qDebug() << "Descriptores hallado: " << fp_template.descriptors.rows;
                //solo admitimos huellas que sean suficientemente buenas
                if(fp_template.descriptors.rows > 0)
                {
                    //guardamos el descriptor e ingresamos los descriptores a la base de datos
                    db.ingresar_template(fp_template,QString::number(id/8));
                    std::cout << "Huella ingresada" << std::endl;
                }
                else
                {
                    std:: cout << "Huella no ingresada" << std::endl;
                }
            }
            id++;
        }

    }
}

//funcion que calcula el false accept rate
//genuine_id indica el id genuino de la huella, no se testea contra este id
double Tester::test_far(const FingerprintTemplate &query_template, const QString &genuine_id, const Database &db)
{
    double far = 0.0;
    //obtenemos una lista con los id de la base de datos
    std::vector<QString> lista_id;
    lista_id = db.obtener_lista_id();
    //para cada id, realizamos la verificacion
    int testeos = 0;
    int aceptados = 0;
    int rechazados = 0;
    for(const QString &id : lista_id)
    {
        if(id != genuine_id)
        {
            //obtenemos la lista de descriptores de la base de datos
            std::vector<fp::FingerprintTemplate> train_templates = db.recuperar_template(id);
            //verificamos
            for(const fp::FingerprintTemplate &fp : train_templates)
            {
                testeos++;
                bool aceptado = comparator.compare(query_template, fp);
                if(aceptado)
                {
                    aceptados++;
                }
                else
                {
                    rechazados++;
                }
            }
        }
    }
    far = (double)aceptados/(double)testeos;
    return far;
}

//funcion que calcula el false reject rate
//genuine_id indica el id genuino de la huella, se testea solo contra este id
double Tester::test_frr(const FingerprintTemplate &query_template, const QString &genuine_id, const Database &db)
{
    double frr = 0.0;
    //obtenemos una lista con los id de la base de datos
    std::vector<QString> lista_id;
    lista_id = db.obtener_lista_id();
    //para cada id, realizamos la verificacion
    int testeos = 0;
    int aceptados = 0;
    int rechazados = 0;
    for(const QString &id : lista_id)
    {
        if(id == genuine_id)
        {
            //obtenemos la lista de descriptores de la base de datos
            std::vector<fp::FingerprintTemplate> train_templates = db.recuperar_template(id);
            //verificamos
            for(const fp::FingerprintTemplate &fp : train_templates)
            {
                testeos++;
                bool aceptado = comparator.compare(query_template, fp);
                if(aceptado)
                {
                    aceptados++;
                }
                else
                {
                    rechazados++;
                }
            }
        }
    }
    frr = (double)rechazados/(double)testeos;
    return frr;
}

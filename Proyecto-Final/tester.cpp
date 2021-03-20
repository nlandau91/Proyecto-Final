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
double Tester::test_far(const Database &db)
{
    double far = 0.0;
    //obtenemos una lista con los id de la base de datos
    std::vector<QString> lista_id;
    lista_id = db.obtener_lista_id();
    //para cada id, realizamos la verificacion
    int testeos = 0;
    int aceptados = 0;
    int rechazados = 0;
    for(size_t i = 0; i < lista_id.size()-1; i++)
    {
        QString genuine_id = lista_id[i];
        std::vector<fp::FingerprintTemplate> genuine_templates = db.recuperar_template(genuine_id);
        for(size_t j = i+1; j < lista_id.size(); j++)
        {
            QString impostor_id = lista_id[j];
            std::vector<fp::FingerprintTemplate> impostor_templates = db.recuperar_template(impostor_id);
            for(fp::FingerprintTemplate genuine_template : genuine_templates)
            {
                for(fp::FingerprintTemplate impostor_template : impostor_templates)
                {
                    testeos++;
                    bool aceptado = comparator.compare(genuine_template, impostor_template);
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
            std::cout << "Test: " << (double)testeos / (2022.40) << "%" << std::endl;
            std::cout << "Test: " << 100.0*(double)aceptados/(double)testeos << "% far" << std::endl;
        }
    }

    far = (double)aceptados/(double)testeos;
    return far;
}

//funcion que calcula el false reject rate
//genuine_id indica el id genuino de la huella, se testea solo contra este id
double Tester::test_frr(const Database &db)
{
    double frr = 0.0;
    //obtenemos una lista con los id de la base de datos
    std::vector<QString> lista_id;
    lista_id = db.obtener_lista_id();
    //para cada id, realizamos la verificacion
    int testeos = 0;
    int aceptados = 0;
    int rechazados = 0;


    for(const QString &genuine_id : lista_id)
    {
        std::vector<fp::FingerprintTemplate> genuine_templates = db.recuperar_template(genuine_id);
        for(size_t i = 0; i < genuine_templates.size(); i++)
        {
            FingerprintTemplate fp_template_1 = genuine_templates[i];
            for(size_t j = i; j < genuine_templates.size(); j++)
            {
                FingerprintTemplate fp_template_2 = genuine_templates[j];
                testeos++;
                bool aceptado = comparator.compare(fp_template_1, fp_template_2);
                if(aceptado)
                {
                    aceptados++;
                }
                else
                {
                    rechazados++;
                }
            }
            std::cout << "Test: " << (double)testeos / (28.80) << "%" << std::endl;
            std::cout << "Test: " << 100.0*(double)rechazados/(double)testeos << "% frr" << std::endl;
        }

    }
    frr = (double)aceptados/(double)testeos;
    return frr;
}

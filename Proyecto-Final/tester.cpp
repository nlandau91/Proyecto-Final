#include "tester.h"
#include "stats.h"

#include <QDebug>
#include <QElapsedTimer>

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
std::vector<double> Tester::test_far(const Database &db)
{
    //double far = 0.0;
    //obtenemos una lista con los id de la base de datos
    std::vector<QString> lista_id;
    lista_id = db.obtener_lista_id();
    //para cada id, realizamos la verificacion
    int testeos = 0;
    //int aceptados = 0;
    //int rechazados = 0;
    std::vector<double> scores = {0};
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
                    //bool aceptado = false;
                    //aceptado = comparator.compare(genuine_template, impostor_template);
                    double score = comparator.compare(genuine_template, impostor_template);
                    scores.push_back(score);
                    //                    aceptado = comparator.match_threshold < score;
                    //                    if(aceptado)
                    //                    {
                    //                        aceptados++;
                    //                    }
                    //                    else
                    //                    {
                    //                        rechazados++;
                    //                    }
                }
            }
            std::cout << "Test far: " << (double)testeos / (2022.40) << "%" << std::endl;
            //std::cout << "Test: " << 100.0*(double)aceptados/(double)testeos << "% far" << std::endl;
        }
    }
    //far = (double)aceptados/(double)testeos;
    return scores;
}

//funcion que calcula el false reject rate
//genuine_id indica el id genuino de la huella, se testea solo contra este id
std::vector<double> Tester::test_frr(const Database &db)
{
    //double frr = 0.0;
    //obtenemos una lista con los id de la base de datos
    std::vector<QString> lista_id;
    lista_id = db.obtener_lista_id();
    //para cada id, realizamos la verificacion
    int testeos = 0;
    //    int aceptados = 0;
    //    int rechazados = 0;

    std::vector<double> scores = {0};
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
                //bool aceptado = false;
                double score = comparator.compare(fp_template_1, fp_template_2);
                scores.push_back(score);
                //                aceptado = comparator.match_threshold < score;
                //                if(aceptado)
                //                {
                //                    aceptados++;
                //                }
                //                else
                //                {
                //                    rechazados++;
                //                }
            }
        }
        std::cout << "Test frr: " << (double)testeos / (28.80) << "%" << std::endl;
        //std::cout << "Test: " << 100.0*(double)rechazados/(double)testeos << "% frr" << std::endl;
    }
    //frr = (double)aceptados/(double)testeos;
    return scores;
}

void Tester::perform_tests(const std::vector<std::vector<double>> &params_list, Database &db)
{
    QString filename = "../tests/Data.csv";
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream stream(&file);

        stream << "ransac_trans"<<"\t"
               << "ransac_thresh"<<"\t"
               << "ransac_iter"<<"\t"
               << "ransac_conf"<<"\t"
               << "median_rat"<<"\t"
               << "sing_compare"<<"\t"
               << "gen_mean"<<"\t"
               << "gen_low"<<"\t"
               << "imp_mean"<<"\t"
               << "imp_high"<<"\t"
               << "eer" << "\t"
               << "time(ms)" << "\n";
        file.close();
    }

    int test_number = 0;
    for(const std::vector<double> &params : params_list)
    {
        std::cout << "Test number: " << test_number << "..." << std::endl;
        double med_th = params[0];
        int ran_trans = (int)params[1];
        double ran_th = params[2];
        double ran_conf = params[3];
        int ran_iter = (int)params[4];;
        comparator.median_threshold = med_th;
        comparator.ransac_transform = ran_trans;
        comparator.ransac_threshold = ran_th;
        comparator.ransac_conf = ran_conf;
        comparator.ransac_iter = ran_iter;

        QElapsedTimer timer;
        timer.start();
        std::vector<double> scores_genuine = test_frr(db);
        std::vector<double> scores_impostor = test_far(db);
        double ms = timer.elapsed();

        double gen_mean = fp::get_mean(scores_genuine,true);
        double gen_lo = fp::get_low_pcnt(scores_genuine,0.05);
        double imp_mean = fp::get_mean(scores_impostor,true);
        double imp_hi = fp::get_high_pcnt(scores_impostor,0.05);
        double eer = fp::get_eer(scores_genuine,scores_impostor);

        QString ran_trans_string;
        if(ran_trans == HOMOGRAPHY)
            ran_trans_string = "HOMOGRAPHY";
        if(ran_trans == AFFINE)
            ran_trans_string = "AFFINE";
        if(ran_trans == PARTIALAFFINE)
            ran_trans_string = "PARTIALAFFINE";

        //QString filename = "../tests/Data.csv";
        //QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            QTextStream stream(&file);

            stream << ran_trans_string <<"\t"
                   << ran_th<<"\t"
                   << ran_iter<<"\t"
                   << ran_conf<<"\t"
                   << med_th<<"\t"
                   << true<<"\t"
                   << gen_mean<<"\t"
                   << gen_lo<<"\t"
                   << imp_mean<<"\t"
                   << imp_hi<<"\t"
                   << eer << "\t"
                   << ms << "\n";
        }
        test_number++;
    }
}

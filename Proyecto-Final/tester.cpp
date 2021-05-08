#include "tester.h"
#include "stats.h"
#include "appsettings.h"
#include "omp.h"

#include <opencv2/imgcodecs.hpp>

#include <iostream>
#include <QTextStream>
#include <QElapsedTimer>

using namespace fp;
Tester::Tester()
{

}

//arma una base de datos para realizar pruebas a partir de una muestra de huellas
void Tester::load_database(fp::Database &db, const QString &path)
{
    //cargamos las carpetas que contienen huellas
    //std::cout << path.toStdString() << std::endl;
    //para cada carpeta, cargamos todas las huellas
    QDir directory(path);
    QStringList fileNames = directory.entryList(QStringList() << "*.tif",QDir::Files);
    for(const QString &fileName : fileNames)
    {
        //std::cout << fileName.toStdString() << std::endl;
        //para cada huella, la procesamos y la ingresamos
        std::string abs_path = (path+"/"+fileName).toStdString();
        cv::Mat src = cv::imread(abs_path,cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //preprocesamos la imagen
            fp::Preprocessed pre = preprocesser.preprocess(src);
            //obtenemos los descriptores
            fp::FingerprintTemplate fp_template = analyzer.analize(pre);
            //solo admitimos huellas que sean suficientemente buenas
            if(fp_template.descriptors.rows > 0)
            {
                //guardamos el descriptor e ingresamos los descriptores a la base de datos
                int id;
                id = fileName.split("_").at(0).toInt();
                db.ingresar_template(fp_template,QString::number(id));
                //std::cout << "Huella ingresada" << std::endl;
            }
            else
            {
                std:: cout << "Huella no ingresada" << std::endl;
            }
        }
    }


}

//funcion que calcula el false accept rate
//genuine_id indica el id genuino de la huella, no se testea contra este id
std::vector<double> Tester::test_far(const Database &db)
{
    std::vector<double> scores = {0};
    //obtenemos una lista con los id de la base de datos
    std::vector<QString> lista_id;
    lista_id = db.obtener_lista_id();
    if(lista_id.size() > 0)
    {
        std::vector<std::vector<fp::FingerprintTemplate>> all_templates;
        for(const QString &id : lista_id)
        {
            std::vector<fp::FingerprintTemplate> templates = db.recuperar_template(id);
            all_templates.push_back(templates);
        }
        //para cada id, realizamos la verificacion
        int testeos = 0;


#pragma omp parallel
        {
            std::vector<double> scores_private;
#pragma omp for nowait
            for(size_t i = 0; i < all_templates.size()-1; i++)
            {
                std::vector<fp::FingerprintTemplate> genuine_templates = all_templates[i];
                for(size_t j = i+1; j < all_templates.size(); j++)
                {
                    int n = 0;
                    std::vector<fp::FingerprintTemplate> impostor_templates = all_templates[j];
                    for(fp::FingerprintTemplate genuine_template : genuine_templates)
                    {
                        for(fp::FingerprintTemplate impostor_template : impostor_templates)
                        {
                            n++;
                            double score = comparator.compare(genuine_template, impostor_template);
                            scores_private.push_back(score);
                        }
                    }
#pragma omp critical
                    {
                        testeos += n;
                        //std::cout << "Test far: " << (double)testeos / (2022.40) << "%" << std::endl;
                    }
                }
            }
#pragma omp critical
            scores.insert(scores.end(),scores_private.begin(),scores_private.end());
        }
        //far = (double)aceptados/(double)testeos;
    }
    return scores;
}

//funcion que calcula el false reject rate
//genuine_id indica el id genuino de la huella, se testea solo contra este id
std::vector<double> Tester::test_frr(const Database &db)
{
    std::vector<double> scores = {0};
    //obtenemos una lista con los id de la base de datos
    std::vector<QString> lista_id;
    lista_id = db.obtener_lista_id();
    if(lista_id.size() > 0)
    {
        //para cada id, realizamos la verificacion
        int testeos = 0;
        std::vector<std::vector<fp::FingerprintTemplate>> all_templates;
        for(const QString &genuine_id : lista_id)
        {
            std::vector<fp::FingerprintTemplate> templates = db.recuperar_template(genuine_id);
            all_templates.push_back(templates);
        }
#pragma omp parallel
        {
            std::vector<double> scores_private;
#pragma omp for nowait
            for(std::vector<fp::FingerprintTemplate> &genuine_templates : all_templates)
            {
                int n = 0;
                for(size_t i = 0; i < genuine_templates.size(); i++)
                {
                    FingerprintTemplate fp_template_1 = genuine_templates[i];
                    for(size_t j = i; j < genuine_templates.size(); j++)
                    {
                        FingerprintTemplate fp_template_2 = genuine_templates[j];
                        double score = comparator.compare(fp_template_1, fp_template_2);
                        scores_private.push_back(score);
                        n++;
                    }
                }
#pragma omp critical
                {
                    testeos +=n;
                    //std::cout << "Test frr: " << (double)testeos / (28.80) << "%" << std::endl;
                }
            }
#pragma omp critical
            scores.insert(scores.end(),scores_private.begin(),scores_private.end());
        }
    }
    return scores;
}

void Tester::perform_tests(const std::vector<TesterParameters> &params_list, Database &db)
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
               << "descriptors"<<"\t"
               << "gen_mean"<<"\t"
               << "gen_low"<<"\t"
               << "imp_mean"<<"\t"
               << "imp_high"<<"\t"
               << "eer_pcnt" << "\t"
               << "eer_val" << "\t"
               << "frr" << "\t"
               << "far" << "\t"
               << "time(ms)" << "\n";
        file.close();
    }

    int test_number = 0;
    for(const TesterParameters &params : params_list)
    {
        double med_th = params.med_th;
        int ran_trans = params.ran_trans;
        double ran_th = params.ran_th;
        double ran_conf = params.ran_conf;
        int ran_iter = params.ran_iter;
        double match_threshold = params.match_threshold;
        comparator.median_threshold = med_th;
        comparator.ransac_model = ran_trans;
        comparator.ransac_threshold = ran_th;
        comparator.ransac_conf = ran_conf;
        comparator.ransac_iter = ran_iter;
        comparator.match_threshold = match_threshold;
        comparator.sing_compare = true;

        QElapsedTimer timer;
        timer.start();
        std::vector<double> scores_genuine = test_frr(db);
        std::vector<double> scores_impostor = test_far(db);
        double ms = timer.elapsed();
        if(scores_genuine.size() > 3 && scores_genuine.size() > 3)
        {
            double gen_mean = fp::get_mean(scores_genuine,true);
            double gen_lo = fp::get_low_pcnt(scores_genuine,0.05);
            double imp_mean = fp::get_mean(scores_impostor,true);
            double imp_hi = fp::get_high_pcnt(scores_impostor,0.05);
            std::vector<double> eer_res = fp::get_eer(scores_genuine,scores_impostor);
            double eer_pcnt = eer_res[0];
            double eer_val = eer_res[1];
            double frr = get_frr(scores_genuine,match_threshold);
            double far = get_far(scores_impostor,match_threshold);
            //double frr = get_frr(scores_genuine,eer_val);
            //double far = get_far(scores_impostor,eer_val);

            QString ran_trans_string;
            if(ran_trans == HOMOGRAPHY)
                ran_trans_string = "HOMOGRAPHY";
            if(ran_trans == AFFINE)
                ran_trans_string = "AFFINE";
            if(ran_trans == PARTIALAFFINE)
                ran_trans_string = "PARTIALAFFINE";

            QString filename = "../tests/Data.csv";
            QFile file(filename);
            if (file.open(QIODevice::WriteOnly | QIODevice::Append))
            {
                QTextStream stream(&file);

                stream << ran_trans_string <<"\t"
                       << ran_th<<"\t"
                       << ran_iter<<"\t"
                       << ran_conf<<"\t"
                       << med_th<<"\t"
                       << "250"<<"\t"
                       << gen_mean<<"\t"
                       << gen_lo<<"\t"
                       << imp_mean<<"\t"
                       << imp_hi<<"\t"
                       << eer_pcnt << "\t"
                       << eer_val << "\t"
                       << frr << "\t"
                       << far << "\t"
                       << ms << "\n";
            }
            test_number++;
        }
    }
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configdialog.h"
#include "stats.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <QIntValidator>
#include <QDebug>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug("Setting up UI...");
    ui->setupUi(this);
    ui->lineEdit_id->setValidator(new QIntValidator(0, INT_MAX, this));
    ui->lineEdit_output->setReadOnly(true);
    qDebug("Initializing db...");
    db.init();
    qDebug("Loading settings...");
    load_settings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_ingresar_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Open Image"), "../res/",
                                                          tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    setEnabled(false);
    output_text("Ingresando huellas...");
    int total_samples = fileNames.length();
    int accepted_samples = 0;
    for(const QString &fileName : fileNames)
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //maximo altura 300px
            double scalefactor = (double)400.0/src.rows;
            cv::resize(src,src,cv::Size(),scalefactor,scalefactor,cv::INTER_CUBIC);
            ui->lbl_fp_input->setPixmap(fp::cv_mat_to_qpixmap(src));
            //preprocesamos la imagen
            fp::Preprocessed pre = preprocesser.preprocess(src);
            //obtenemos los descriptores
            fp::FingerprintTemplate fp_template = analyzer.analize(pre);
            qDebug() << "Descriptores hallado: " << fp_template.descriptors.rows;
            //dibujamos sobre la imagen de salida
            cv::Mat enhanced_marked;
            cv::cvtColor(pre.result,enhanced_marked,cv::COLOR_GRAY2BGR);
            if(app_settings.draw_over_output)
            {
                enhanced_marked = fp::draw_keypoints(enhanced_marked,fp_template.keypoints);
                enhanced_marked = fp::draw_keypoints(enhanced_marked,fp_template.minutiaes);
                enhanced_marked = fp::draw_singularities(enhanced_marked,fp_template.singularities);
            }
            ui->lbl_fp_output->setPixmap(fp::cv_mat_to_qpixmap(enhanced_marked));
            //solo admitimos huellas que sean suficientemente buenas
            if(fp_template.descriptors.rows > 4)
            {
                //guardamos el descriptor e ingresamos los descriptores a la base de datos
                QString id = ui->lineEdit_id->text();
                //db.ingresar_descriptores(analysis.descriptors,id);
                if(db.ingresar_template(fp_template,id))
                {
                    accepted_samples++;
                }
                else
                {
                    output_text("No se puedo ingresar la huella a la base de datos.");
                }

                //std::cout << "Huella ingresada" << std::endl;
            }
            else
            {
                //std:: cout << "Huella no ingresada" << std::endl;
            }
        }
    }
    output_text("Huellas ingresadas: "+QString::number(accepted_samples)+" de "+QString::number(total_samples));
    setEnabled(true);
}

void MainWindow::on_btn_verificar_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "../res/",
                                                    tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    setEnabled(false);
    output_text("Verificando...");
    bool verificado = false;
    if(!fileName.isEmpty())
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //maximo altura 300px
            double scalefactor = (double)400.0/src.rows;
            cv::resize(src,src,cv::Size(),scalefactor,scalefactor,cv::INTER_CUBIC);
            ui->lbl_fp_input->setPixmap(fp::cv_mat_to_qpixmap(src));
            //preprocesamos la imagen
            fp::Preprocessed pre = preprocesser.preprocess(src);
            //obtenemos los descriptores
            fp::FingerprintTemplate fp_template = analyzer.analize(pre);
            //dibujamos sobre la imagen de salida
            cv::Mat enhanced_marked;
            cv::cvtColor(pre.result,enhanced_marked,cv::COLOR_GRAY2BGR);
            if(app_settings.draw_over_output)
            {
                enhanced_marked = fp::draw_keypoints(enhanced_marked,fp_template.keypoints);
                enhanced_marked = fp::draw_keypoints(enhanced_marked,fp_template.minutiaes);
                enhanced_marked = fp::draw_singularities(enhanced_marked,fp_template.singularities);
            }
            ui->lbl_fp_output->setPixmap(fp::cv_mat_to_qpixmap(enhanced_marked));
            //solo admitimos huellas que sean suficientemente buenas
            if(fp_template.descriptors.rows > 4)
            {
                //obtenemos la lista de descriptores de la base de datos
                QString id = ui->lineEdit_id->text();
                std::vector<fp::FingerprintTemplate> train_templates = db.recuperar_template(id);
                //verificamos
                for(fp::FingerprintTemplate train_template : train_templates)
                {
                    verificado = verificado || comparator.match_threshold <= comparator.compare(fp_template, train_template);
                }
            }

        }
    }
    output_text("Verificacion: "+QVariant(verificado).toString());
    setEnabled(true);
}

void MainWindow::on_btn_identificar_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "../res/",
                                                    tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    setEnabled(false);
    output_text("Identificando...");
    QString best_id = "ninguno";
    if(!fileName.isEmpty())
    {
        setEnabled(false);
        //leemos la imagen en escala de gris
        qDebug() << fileName;
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //maximo altura 300px
            double scalefactor = (double)400.0/src.rows;
            cv::resize(src,src,cv::Size(),scalefactor,scalefactor,cv::INTER_CUBIC);
            ui->lbl_fp_input->setPixmap(fp::cv_mat_to_qpixmap(src));
            //preprocesamos la imagen
            fp::Preprocessed pre = preprocesser.preprocess(src);
            //obtenemos los descriptores
            fp::FingerprintTemplate fp_template = analyzer.analize(pre);
            //dibujamos sobre la imagen de salida
            cv::Mat enhanced_marked;
            cv::cvtColor(pre.result,enhanced_marked,cv::COLOR_GRAY2BGR);
            if(app_settings.draw_over_output)
            {
                enhanced_marked = fp::draw_keypoints(enhanced_marked,fp_template.keypoints);
                enhanced_marked = fp::draw_keypoints(enhanced_marked,fp_template.minutiaes);
                enhanced_marked = fp::draw_singularities(enhanced_marked,fp_template.singularities);
            }
            ui->lbl_fp_output->setPixmap(fp::cv_mat_to_qpixmap(enhanced_marked));
            //solo admitimos huellas que sean suficientemente buenas
            if(fp_template.descriptors.rows > 4)
            {
                //obtenemos una lista con los id de la base de datos
                std::vector<QString> lista_id;
                lista_id = db.obtener_lista_id();
                //buscamos el mejor score entre todos los id
                double best_score = 0.0;
                for(const QString &id : lista_id)
                {
                    //obtenemos la lista de descriptores de la base de datos
                    std::vector<fp::FingerprintTemplate> train_templates = db.recuperar_template(id);
                    //buscamos el mejor score entre los templates del id
                    double id_score = 0.0;

                    for(const fp::FingerprintTemplate &fp : train_templates)
                    {
                        double template_score = comparator.compare(fp_template, fp);
                        if(template_score > app_settings.matcher_threshold && template_score > id_score)
                        {
                            id_score = template_score;
                        }
                    }
                    if(id_score > best_score)
                    {
                        best_score = id_score;
                        best_id = id;
                    }
                    if(comparator.match_threshold <= id_score)
                    {
                        //std::cout << "Posible match id: " << id.toStdString() << " score: " << id_score << std::endl;
                    }
                }
                if(comparator.match_threshold <= best_score)
                {
                    //std::cout << "Mejor match id: " << best_id.toStdString() << " score: " << best_score << std::endl;
                }
            }
        }
    }
    output_text("Identificacion: " + best_id);
    setEnabled(true);
}

void MainWindow::load_settings()
{
    app_settings.load_settings();
    preprocesser = fp::Preprocesser();
    preprocesser.enhancement_method = app_settings.enhancement_method;
    preprocesser.thinning_method = app_settings.thinning_method;
    preprocesser.segment = app_settings.segment;
    preprocesser.blk_sze = app_settings.blk_size;
    preprocesser.blk_orient = app_settings.blk_orient;
    preprocesser.blk_freq = app_settings.blk_freq;
    preprocesser.blk_mask = app_settings.blk_segm;
    preprocesser.norm_req_mean = 100;
    preprocesser.norm_req_var = 100;
    preprocesser.roi_threshold_ratio = app_settings.roi_threshold;
    preprocesser.gabor_kx = app_settings.gabor_kx;
    preprocesser.gabor_ky = app_settings.gabor_ky;


    analyzer = fp::Analyzer();
    analyzer.l1_features_method = fp::POINCARE;
    analyzer.keypoint_method = app_settings.keypoint_detector;
    analyzer.keypoint_threshold = app_settings.keypoint_threshold;
    analyzer.descriptor_method = app_settings.descriptor_method;
    analyzer.blk_sze = app_settings.blk_size;
    analyzer.poincare_tol = 1;

    comparator = fp::Comparator();
    comparator.matcher_method = app_settings.descriptor_method;
    comparator.match_threshold = app_settings.matcher_threshold;
    comparator.ransac_model = app_settings.ransac_model;
    comparator.ransac_threshold = app_settings.ransac_threshold;
    comparator.median_threshold = app_settings.median_threshold;
    comparator.ransac_conf = app_settings.ransac_conf;
    comparator.ransac_iter = app_settings.ransac_iter;
    comparator.sing_compare = true;

    tester.preprocesser = preprocesser;
    tester.analyzer = analyzer;
    tester.comparator = comparator;
}

void MainWindow::on_actionOpciones_triggered()
{
    ConfigDialog config_dialog;
    config_dialog.setModal(true);
    int result = config_dialog.exec();
    if(result == QDialog::Accepted)
    {
        load_settings();
    }
}

void MainWindow::on_btn_demo_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "../res/",
                                                    tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    setEnabled(false);
    output_text("Analizando...");
    //ui->lbl_fp_output->clear();
    if(!fileName.isEmpty())
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //maximo altura
            double scalefactor = (double)400.0/src.rows;
            cv::resize(src,src,cv::Size(),scalefactor,scalefactor,cv::INTER_CUBIC);
            ui->lbl_fp_input->setPixmap(fp::cv_mat_to_qpixmap(src));
            //preprocesamos la imagen
            fp::Preprocessed pre = preprocesser.preprocess(src);
            //obtenemos los descriptores
            fp::FingerprintTemplate fp_template = analyzer.analize(pre);

            //dibujamos sobre la imagen de salida
            cv::Mat enhanced_marked;
            cv::cvtColor(pre.result,enhanced_marked,cv::COLOR_GRAY2BGR);
            if(app_settings.draw_over_output)
            {

                enhanced_marked = fp::draw_keypoints(enhanced_marked,fp_template.minutiaes);
                enhanced_marked = fp::draw_keypoints(enhanced_marked,fp_template.keypoints);
                enhanced_marked = fp::draw_singularities(enhanced_marked,fp_template.singularities);
            }
            ui->lbl_fp_output->setPixmap(fp::cv_mat_to_qpixmap(enhanced_marked));
            output_window = new OutputWindow();
            output_window->setup(fp_template, pre);
            output_window->show();

            //guardamos las imagenes del proceso
            QDir qdir = QDir::current();
            qdir.mkdir("output");
            cv::imwrite("output/normalized.jpg",pre.normalized);
            cv::imwrite("output/roi.jpg",pre.roi);
            cv::imwrite("output/orientation.jpg",fp::visualize_angles(pre.result,pre.orientation));
            cv::imwrite("output/frequency.jpg",fp::visualize_frequencies(pre.result,pre.frequency));
            cv::imwrite("output/filtered.jpg",pre.filtered);
            cv::imwrite("output/thinned.jpg",pre.thinned);
            cv::imwrite("output/preprocessed.jpg",pre.result);
            cv::imwrite("output/output_marked.jpg",enhanced_marked);
            cv::imwrite("output/sharpened.jpg",pre.grayscale);
        }
    }
    output_text("Analisis completo.");
    setEnabled(true);
}



void MainWindow::on_btn_far_clicked()
{
    output_text("Testeando FAR, esto puede tardar varios minutos...");

    std::vector<double> scores = tester.test_far(db);
    double far = 100 * fp::get_far(scores,app_settings.matcher_threshold);
    output_text("Resultado Test FAR: " + QString::number(far,'f',2) + "%");
}

void MainWindow::on_btn_frr_clicked()
{
    output_text("Testeando FRR, esto puede tardar varios minutos...");

    std::vector<double> scores = tester.test_frr(db);
    double frr = 100 * fp::get_frr(scores,app_settings.matcher_threshold);
    output_text("Resultado Test FRR: " + QString::number(frr,'f',2) + "%");
}
void MainWindow::load_database(const QString &path)
{
    //cargamos las carpetas que contienen huellas
    //para cada carpeta, cargamos todas las huellas
    QDir directory(path);
    QStringList fileNames = directory.entryList(QStringList() << "*.tif",QDir::Files);
    for(const QString &fileName : fileNames)
    {
        qDebug() << "Ingresando huella: " + fileName;
        //para cada huella, la procesamos y la ingresamos
        std::string abs_path = (path+"/"+fileName).toStdString();
        cv::Mat src = cv::imread(abs_path,cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //maximo altura
            //double scalefactor = (double)400.0/src.rows;
            //cv::resize(src,src,cv::Size(),scalefactor,scalefactor,cv::INTER_CUBIC);
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
                output_text("Ingresada huella " + fileName);
            }
        }
    }


}
void MainWindow::on_btn_db_clicked()
{
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Carga por lote"),
                                                  tr("Las huellas deben tener nombre '<id>_*.tif'. Continuar?")))
    {
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::DirectoryOnly);
        if(dialog.exec() == QDialog::Accepted)
        {
            output_text("Cargando DB por lotes, esto puede tardar varios minutos...");

            QString path = dialog.directory().absolutePath();
            load_database(path);
            output_text("Carga DB por lotes completa!");
        }
    }
}

void MainWindow::on_btn_fullTest_clicked()
{
    output_text("Testeando FAR y FRR, esto puede tardar varios minutos...");
    //creamos la lista de parametros
    std::vector<fp::Tester::TesterParameters> params_list;

    fp::Tester::TesterParameters tester_parameters;
    tester_parameters.med_th=app_settings.median_threshold;
    tester_parameters.ran_trans=app_settings.ransac_model;
    tester_parameters.ran_th=app_settings.ransac_threshold;
    tester_parameters.ran_conf=app_settings.ransac_conf;
    tester_parameters.ran_iter=app_settings.ransac_iter;
    tester_parameters.match_threshold=app_settings.matcher_threshold;
    params_list.push_back(tester_parameters);
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,2.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,2.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,2.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,3.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,3.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,3.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,4.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,4.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,4.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,2.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,2.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,2.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,3.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,3.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,3.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,4.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,4.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,4.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,2.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,2.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,2.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,3.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,3.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,3.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,4.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,4.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,4.0,0.995,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,2.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,2.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,2.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,3.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,3.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,3.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,4.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,4.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,4.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,2.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,2.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,2.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,3.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,3.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,3.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,4.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,4.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,4.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,2.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,2.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,2.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,3.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,3.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,3.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,4.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,4.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,4.0,0.995,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,2.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,2.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,2.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,3.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,3.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,3.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,4.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,4.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,4.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,2.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,2.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,2.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,3.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,3.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,3.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,4.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,4.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,4.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,2.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,2.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,2.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,3.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,3.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,3.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,4.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,4.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,4.0,0.99,2000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,2.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,2.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,2.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,3.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,3.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,3.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::HOMOGRAPHY,4.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::HOMOGRAPHY,4.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::HOMOGRAPHY,4.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,2.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,2.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,2.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,3.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,3.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,3.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::AFFINE,4.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::AFFINE,4.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::AFFINE,4.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,2.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,2.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,2.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,3.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,3.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,3.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.5,fp::PARTIALAFFINE,4.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{2.0,fp::PARTIALAFFINE,4.0,0.99,1000,app_settings.matcher_threshold});
    params_list.push_back(fp::Tester::TesterParameters{1.5,fp::PARTIALAFFINE,4.0,0.99,1000,app_settings.matcher_threshold});

    tester.perform_tests(params_list, db);
    output_text("Test completo, resultados guardados.");
    load_settings();
}
void MainWindow::output_text(const QString &text)
{
    ui->lineEdit_output->setText(text);
    ui->lineEdit_output->repaint();
}

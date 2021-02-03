#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configdialog.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug("Setting up UI...");
    ui->setupUi(this);
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
    for(QString fileName : fileNames)
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            ui->lbl_fp_input->setPixmap(fp::cvMatToQPixmap(src));
            //preprocesamos la imagen
            fp::Preprocessed pre = preprocesser.preprocess(src);
            //obtenemos los descriptores
            fp::Analysis analysis = analyzer.analize(pre);
            qDebug() << "Descriptores hallado: " << analysis.descriptors.rows;
            //dibujamos sobre la imagen de salida
            cv::Mat enhanced_marked;
            cv::cvtColor(pre.result,enhanced_marked,cv::COLOR_GRAY2BGR);
            if(app_settings.draw_over_output)
            {
                enhanced_marked = fp::draw_minutiae(enhanced_marked,analysis.l2_features);
                enhanced_marked = fp::draw_singularities(enhanced_marked,analysis.l1_features);
            }
            ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced_marked));
            //solo admitimos huellas que sean suficientemente buenas
            if(analysis.descriptors.rows > 4)
            {
                //guardamos el descriptor e ingresamos los descriptores a la base de datos
                QString id = ui->lineEdit->text();
                db.ingresar_descriptor(analysis.descriptors,id);
                std::cout << "Huella ingresada" << std::endl;
            }
            else
            {
                std:: cout << "Huella no ingresada" << std::endl;
            }

        }
    }
    setEnabled(true);
}

void MainWindow::on_btn_verificar_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "../res/",
                                                    tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    if(!fileName.isEmpty())
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            ui->lbl_fp_input->setPixmap(fp::cvMatToQPixmap(src));
            //preprocesamos la imagen
            fp::Preprocessed pre = preprocesser.preprocess(src);
            //obtenemos los descriptores
            fp::Analysis analysis = analyzer.analize(pre);
            qDebug() << "Descriptores hallado: " << analysis.descriptors.rows;
            //dibujamos sobre la imagen de salida
            cv::Mat enhanced_marked;
            cv::cvtColor(pre.result,enhanced_marked,cv::COLOR_GRAY2BGR);
            if(app_settings.draw_over_output)
            {
                enhanced_marked = fp::draw_minutiae(enhanced_marked,analysis.l2_features);
                enhanced_marked = fp::draw_singularities(enhanced_marked,analysis.l1_features);
            }
            ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced_marked));
            //solo admitimos huellas que sean suficientemente buenas
            bool verificado = false;
            if(analysis.descriptors.rows > 0)
            {
                //obtenemos la lista de descriptores de la base de datos
                QString id = ui->lineEdit->text();
                std::vector<cv::Mat> lista_descriptores;
                lista_descriptores = db.obtener_lista_descriptores(id);
                //verificamos
                verificado = 80>=comparator.compare(analysis.descriptors, lista_descriptores);
            }
            if(verificado)
            {
                std::cout << "Verificado!" << std::endl;
            }
            else
            {
                std::cout << "NO Verificado!" << std::endl;
            }
        }
    }
}

void MainWindow::on_btn_identificar_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "../res/",
                                                    tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    if(!fileName.isEmpty())
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            ui->lbl_fp_input->setPixmap(fp::cvMatToQPixmap(src));
            //preprocesamos la imagen
            fp::Preprocessed pre = preprocesser.preprocess(src);
            //obtenemos los descriptores
            fp::Analysis analysis = analyzer.analize(pre);
            qDebug() << "Descriptores hallado: " << analysis.descriptors.rows;
            //dibujamos sobre la imagen de salida
            cv::Mat enhanced_marked;
            cv::cvtColor(pre.result,enhanced_marked,cv::COLOR_GRAY2BGR);
            if(app_settings.draw_over_output)
            {
                enhanced_marked = fp::draw_minutiae(enhanced_marked,analysis.l2_features);
                enhanced_marked = fp::draw_singularities(enhanced_marked,analysis.l1_features);
            }
            ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced_marked));
            //solo admitimos huellas que sean suficientemente buenas
            if(analysis.descriptors.rows > 0)
            {
                //obtenemos una lista con los id de la base de datos
                std::vector<QString> lista_id;
                lista_id = db.obtener_lista_id();
                //para cada id, realizamos la verificacion
                bool verificado = false;
                QString best_id;
                for(QString id : lista_id)
                {
                    //obtenemos la lista de descriptores de la base de datos
                    std::vector<cv::Mat> lista_descriptores;
                    lista_descriptores = db.obtener_lista_descriptores(id);
                    //obtenemos el mejor resultado entre los match de los descriptores
                    verificado = 80>=comparator.compare(analysis.descriptors, lista_descriptores);
                    if(verificado)
                    {
                        std::cout << "Match encontrado: " << id.toStdString() << std::endl;;
                    }

                }
                if(!verificado)
                {
                    std::cout << "No verificado" << std::endl;
                }
            }
            else
            {
                std::cout << "Ingrese la huella nuevamente" << std::endl;
            }
        }
    }
}

void MainWindow::load_settings()
{
    app_settings.load_settings();
    preprocesser = fp::Preprocesser(app_settings.enhancement_method,app_settings.thinning_method);
    preprocesser.segment = app_settings.segment;
    preprocesser.blk_sze = app_settings.blk_size;
    analyzer = fp::Analyzer(app_settings.keypoint_extractor,app_settings.keypoint_threshold,app_settings.feature_extractor,app_settings.matcher_method,app_settings.max_match_dist);
    comparator = fp::Comparator();
    comparator.matcher_method = app_settings.feature_extractor;
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
    if(!fileName.isEmpty())
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            ui->lbl_fp_input->setPixmap(fp::cvMatToQPixmap(src));
            //preprocesamos la imagen
            fp::Preprocessed pre = preprocesser.preprocess(src);
            //obtenemos los descriptores
            fp::Analysis analysis = analyzer.analize(pre);
            qDebug() << "Descriptores hallados: " << analysis.descriptors.rows;
            //dibujamos sobre la imagen de salida
            cv::Mat enhanced_marked;
            cv::cvtColor(pre.result,enhanced_marked,cv::COLOR_GRAY2BGR);
            enhanced_marked = fp::draw_minutiae(enhanced_marked,analysis.l2_features);
            enhanced_marked = fp::draw_singularities(enhanced_marked,analysis.l1_features);

            ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced_marked));

            QDir qdir = QDir::current();
            qdir.mkdir("output");
            cv::imwrite("output/normalized.jpg",pre.normalized);
            cv::imwrite("output/roi.jpg",pre.roi);
            cv::imwrite("output/orientation.jpg",fp::visualize_angles(pre.result,pre.orientation));;
            cv::imwrite("output/filtered.jpg",pre.filtered);
            cv::imwrite("output/thinned.jpg",pre.thinned);
            cv::imwrite("output/preprocessed.jpg",pre.result);
            cv::imwrite("output/output_marked.jpg",enhanced_marked);

            //solo admitimos huellas que sean suficientemente buenas
            if(analysis.descriptors.rows > 0)
            {
            }
            else
            {
            }
        }
    }
}

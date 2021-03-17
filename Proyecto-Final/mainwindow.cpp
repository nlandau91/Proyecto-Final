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

    bool testing = true;
    if(testing)
    {
        fp::Tester tester;
        tester.preprocesser = preprocesser;
        tester.analyzer = analyzer;
        tester.comparator = comparator;
        tester.load_database(db);
        //double far = tester.test_far(db);
        //double frr = tester.test_frr(db);
        //qDebug() << frr;
    }

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
    for(const QString &fileName : fileNames)
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            ui->lbl_fp_input->setPixmap(fp::cvMatToQPixmap(src));
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
            ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced_marked));
            //solo admitimos huellas que sean suficientemente buenas
            if(fp_template.descriptors.rows > 0)
            {
                //guardamos el descriptor e ingresamos los descriptores a la base de datos
                QString id = ui->lineEdit->text();
                //db.ingresar_descriptores(analysis.descriptors,id);
                db.ingresar_template(fp_template,id);
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
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Open Image"), "../res/",
                                                          tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));

    for(const QString &fileName : fileNames)
    {
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
                ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced_marked));
                //solo admitimos huellas que sean suficientemente buenas
                bool verificado = false;
                if(fp_template.descriptors.rows > 0)
                {
                    //obtenemos la lista de descriptores de la base de datos
                    QString id = ui->lineEdit->text();
                    std::vector<fp::FingerprintTemplate> train_templates = db.recuperar_template(id);
                    //verificamos
                    verificado = comparator.compare(fp_template, train_templates);
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
}

void MainWindow::on_btn_identificar_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Open Image"), "../res/",
                                                          tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    for(const QString &fileName : fileNames)
    {
        if(!fileName.isEmpty())
        {
            //leemos la imagen en escala de gris
            qDebug() << fileName;
            cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
            if(!src.empty())
            {
                ui->lbl_fp_input->setPixmap(fp::cvMatToQPixmap(src));
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
                ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced_marked));
                //solo admitimos huellas que sean suficientemente buenas
                if(fp_template.descriptors.rows > 0)
                {
                    //obtenemos una lista con los id de la base de datos
                    std::vector<QString> lista_id;
                    lista_id = db.obtener_lista_id();
                    //para cada id, realizamos la verificacion
                    bool verificado = false;
                    for(const QString &id : lista_id)
                    {
                        //obtenemos la lista de descriptores de la base de datos
                        std::vector<fp::FingerprintTemplate> train_templates = db.recuperar_template(id);
                        //verificamos

                        for(const fp::FingerprintTemplate &fp : train_templates)
                        {
                            verificado = verificado || comparator.compare(fp_template, fp);

                        }
                        if(verificado)
                        {
                            std::cout << "Match encontrado: " << id.toStdString() << std::endl;
                        }
                    }
                }
                else
                {
                    std::cout << "Ingrese la huella nuevamente" << std::endl;
                }
            }
        }
    }
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
    comparator.edge_matching = app_settings.edge_matching;
    comparator.edge_angle = app_settings.edge_angle;
    comparator.edge_dist = app_settings.edge_dist;
    comparator.triangle_max_edge = app_settings.triangle_max_edge;
    comparator.triangle_min_edge = app_settings.triangle_min_edge;
    comparator.ransac_threshold = app_settings.ransac_threshold;
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
            output_window = new OutputWindow();
            output_window->setup(fp_template, enhanced_marked);
            output_window->show();

            ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced_marked));

            //guardamos las imagenes del proceso
            QDir qdir = QDir::current();
            qdir.mkdir("output");
            cv::imwrite("output/normalized.jpg",pre.normalized);
            cv::imwrite("output/roi.jpg",pre.roi);
            cv::imwrite("output/orientation.jpg",fp::visualize_angles(pre.result,pre.orientation));
            cv::imwrite("output/frequency.jpg",pre.frequency * 255);
            cv::imwrite("output/filtered.jpg",pre.filtered);
            cv::imwrite("output/thinned.jpg",pre.thinned);
            cv::imwrite("output/preprocessed.jpg",pre.result);
            cv::imwrite("output/output_marked.jpg",enhanced_marked);

        }
    }
}

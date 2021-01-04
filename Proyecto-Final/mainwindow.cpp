#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug("Setting up UI...");
    ui->setupUi(this);
    qDebug("Initializing db...");
    db.init();
    qDebug("Populating enum parser...");
    enum_parser.add_enum(fp::Preprocesser::NONE,"none");
    enum_parser.add_enum(fp::Preprocesser::GABOR,"gabor");
    fp::Preprocesser::EnhancementMethod averga = (fp::Preprocesser::EnhancementMethod)enum_parser.parse_enum("gab?");
    std::cout << averga << std::endl;
    //enum_parser.add_enum(fp::Preprocesser::EnhancementMethod::GABOR,"asd");
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
    //std::string id = ui->lineEdit->text().toStdString();
    //int n = 0;
    for(QString fileName : fileNames)
    {
        //ui->lineEdit->setText(QString::number(n/8));
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            ui->lbl_fp_input->setPixmap(fp::cvMatToQPixmap(src));
            //mejoramos la imagen
            cv::Mat enhanced = fp::Preprocesser::preprocess(src,fp::Preprocesser::GABOR,fp::Preprocesser::ZHANGSUEN, false);
            //obtenemos los descriptores
            fp::Analyzer::Analysis analysis = fp::Analyzer::analize(enhanced);
            qDebug() << "Descriptores hallado: " << analysis.descriptors.rows;
            //dibujamos sobre la imagen de salida
            cv::Mat enhanced_marked;
            cv::cvtColor(enhanced,enhanced_marked,cv::COLOR_GRAY2BGR);
            cv::drawKeypoints(enhanced_marked,analysis.keypoints,enhanced_marked,cv::Scalar(0,255,0),cv::DrawMatchesFlags::DRAW_OVER_OUTIMG);
            ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced_marked));
            //solo ingresamos huellas que sean suficientemente buenas
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
        //n++;
    }
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
            //mejoramos la imagen
            cv::Mat enhanced = fp::Preprocesser::preprocess(src,fp::Preprocesser::GABOR,fp::Preprocesser::ZHANGSUEN);
            ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced));;
            //obtenemos los descriptores
            fp::Analyzer::Analysis analysis = fp::Analyzer::analize(enhanced);
            //solo verificamos si la huella es buena
            bool verificado = false;
            if(analysis.descriptors.rows > 0)
            {
                //obtenemos la lista de descriptores de la base de datos
                QString id = ui->lineEdit->text();
                std::vector<cv::Mat> lista_descriptores;
                lista_descriptores = db.obtener_lista_descriptores(id);
                //verificamos
                verificado = 80>=fp::Comparator::compare(analysis.descriptors, lista_descriptores);
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
            //mejoramos la imagen
            cv::Mat enhanced = fp::Preprocesser::preprocess(src,fp::Preprocesser::GABOR,fp::Preprocesser::ZHANGSUEN);
            ui->lbl_fp_output->setPixmap(fp::cvMatToQPixmap(enhanced));
            //obtenemos los descriptores
            fp::Analyzer::Analysis analysis = fp::Analyzer::analize(enhanced);
            //solo verificamos si la huella es buena
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
                    verificado = 80>=fp::Comparator::compare(analysis.descriptors, lista_descriptores);
                    if(verificado)
                    {
                        std::cout << "Match encontrado: " << id.toStdString() << std::endl;;
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

void MainWindow::load_settings()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);
    QString enh_method = settings.value("enhance_method").toString();
    QString thi_method = settings.value("thinning_method").toString();
    QString masking = settings.value("masking").toString();


    app_settings.enhancement_method = (fp::Preprocesser::EnhancementMethod)enum_parser.parse_enum(enh_method.toStdString());

    app_settings.thinning_method = (fp::Preprocesser::ThinningMethod)enum_parser.parse_enum(thi_method.toStdString());


}


void MainWindow::on_actionOpciones_triggered()
{
    qDebug() << "Entrando a opciones";
    ConfigDialog config_dialog;
    config_dialog.setModal(true);
    config_dialog.exec();
    qDebug() << "Saliendo de opciones";
    load_settings();
}
